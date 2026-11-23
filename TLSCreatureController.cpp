#include "AI/TLSCreatureController.h"
#include "AI/TLSCreature.h"
#include "Navigation/PathFollowingComponent.h"
#include "Player/TLSCharacter.h"
#include "Player/TLSFlashlightComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ATLSCreatureController::ATLSCreatureController()
{
	PrimaryActorTick.bCanEverTick = true;
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
	SetPerceptionComponent(*PerceptionComp);
	bAttachToPawn = true;
}

void ATLSCreatureController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	Creature = Cast<ATLSCreature>(InPawn);
	if (!Creature)
	{
		return;
	}

	Home = Creature->GetActorLocation();
	PerceptionComp->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &ATLSCreatureController::HandleTargetPerception);
	ChangeState(Creature->bStartDormant ? ETLSAIState::Dormant : ETLSAIState::Patrol);
}

void ATLSCreatureController::ConfigurePerception(ATLSCreature* InCreature)
{
	// Done lazily on the first tick: the creature's per-type BeginPlay tuning has run by then.
	UAISenseConfig_Sight* Sight = NewObject<UAISenseConfig_Sight>(this, TEXT("SightConfig"));
	Sight->SightRadius = InCreature->SightRange;
	Sight->LoseSightRadius = InCreature->SightRange + 300.f;
	Sight->PeripheralVisionAngleDegrees = (InCreature->CreatureType == ETLSCreatureType::Stalker) ? 90.f : 70.f;
	Sight->DetectionByAffiliation.bDetectEnemies = true;
	Sight->DetectionByAffiliation.bDetectNeutrals = true;
	Sight->DetectionByAffiliation.bDetectFriendlies = true;

	UAISenseConfig_Hearing* Hearing = NewObject<UAISenseConfig_Hearing>(this, TEXT("HearingConfig"));
	Hearing->HearingRange = InCreature->HearingRange;
	Hearing->DetectionByAffiliation.bDetectEnemies = true;
	Hearing->DetectionByAffiliation.bDetectNeutrals = true;
	Hearing->DetectionByAffiliation.bDetectFriendlies = true;

	PerceptionComp->ConfigureSense(*Sight);
	PerceptionComp->ConfigureSense(*Hearing);
	PerceptionComp->SetDominantSense(Sight->GetSenseImplementation());

	SightSenseID = UAISense::GetSenseID<UAISense_Sight>();
	HearingSenseID = UAISense::GetSenseID<UAISense_Hearing>();
	bConfigured = true;
}

void ATLSCreatureController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!Creature)
	{
		Creature = Cast<ATLSCreature>(GetPawn());
		if (!Creature)
		{
			return;
		}
	}
	if (!bConfigured)
	{
		ConfigurePerception(Creature);
	}

	ATLSCharacter* Player = Target.Get();
	if (!Player)
	{
		Player = Cast<ATLSCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	}

	// Flashlight recoil: sustained light on the creature pushes it back. Hunters ignore light.
	const bool bLit = Player && Creature->CreatureType != ETLSCreatureType::Hunter
		&& IsPlayerLightingCreature(Player, Creature);
	if (bLit)
	{
		LightExposure += DeltaSeconds;
		if (LightExposure >= Creature->LightRecoilThreshold && State != ETLSAIState::Retreat)
		{
			Creature->PlayRecoil();
			ChangeState(ETLSAIState::Retreat);
		}
	}
	else
	{
		LightExposure = FMath::Max(0.f, LightExposure - DeltaSeconds * 0.5f);
	}

	if (Player)
	{
		Player->SetHunted(State == ETLSAIState::Hunt);
	}

	TickState(DeltaSeconds);
}

void ATLSCreatureController::TickState(float DeltaSeconds)
{
	switch (State)
	{
	case ETLSAIState::Dormant:
		// Waiting. Perception handlers wake it.
		break;

	case ETLSAIState::Patrol:
		if (GetMoveStatus() == EPathFollowingStatus::Idle)
		{
			StateTimer -= DeltaSeconds;
			if (StateTimer <= 0.f)
			{
				MoveToRandomPointNear(Home, Creature->PatrolRadius, Creature->PatrolWalkSpeed);
				StateTimer = FMath::RandRange(2.f, 5.f);
			}
		}
		break;

	case ETLSAIState::Investigate:
		if (!bMoveIssued)
		{
			Creature->GetCharacterMovement()->MaxWalkSpeed = Creature->InvestigateSpeed;
			MoveToLocation(NoiseLocation, 120.f);
			bMoveIssued = true;
		}
		else if (GetMoveStatus() == EPathFollowingStatus::Idle)
		{
			StateTimer -= DeltaSeconds; // Looks around for a moment at the source.
			if (StateTimer <= 0.f)
			{
				ChangeState(Creature->bStartDormant ? ETLSAIState::Dormant : ETLSAIState::Patrol);
			}
		}
		break;

	case ETLSAIState::Hunt:
		TickHunt(DeltaSeconds);
		break;

	case ETLSAIState::Search:
		StateTimer -= DeltaSeconds;
		if (!bMoveIssued)
		{
			Creature->GetCharacterMovement()->MaxWalkSpeed = Creature->InvestigateSpeed;
			MoveToLocation(LastKnownLocation, 120.f);
			bMoveIssued = true;
		}
		else
		{
			// Sweep the area: once idle, pick a nearby spot every few seconds until the search times out.
			RepathTimer -= DeltaSeconds;
			if (GetMoveStatus() == EPathFollowingStatus::Idle && RepathTimer <= 0.f)
			{
				MoveToRandomPointNear(LastKnownLocation, 900.f, Creature->InvestigateSpeed);
				RepathTimer = 2.5f;
			}
		}
		if (StateTimer <= 0.f)
		{
			ChangeState(Creature->bStartDormant ? ETLSAIState::Dormant : ETLSAIState::Patrol);
		}
		break;

	case ETLSAIState::Retreat:
		StateTimer -= DeltaSeconds;
		if (!bMoveIssued)
		{
			const ATLSCharacter* Player = Target.Get();
			if (Player)
			{
				const FVector Away = (Creature->GetActorLocation() - Player->GetActorLocation()).GetSafeNormal2D();
				MoveToRandomPointNear(Creature->GetActorLocation() + Away * 1500.f, 600.f, Creature->PatrolWalkSpeed);
			}
			bMoveIssued = true;
		}
		if (StateTimer <= 0.f)
		{
			LastKnownLocation = Target.IsValid() ? Target->GetActorLocation() : LastKnownLocation;
			ChangeState(ETLSAIState::Search);
		}
		break;
	}
}

void ATLSCreatureController::TickHunt(float DeltaSeconds)
{
	ATLSCharacter* Player = Target.Get();
	if (!Player || Player->IsDead())
	{
		ChangeState(ETLSAIState::Search);
		return;
	}

	if (bSightContact)
	{
		LastKnownLocation = Player->GetActorLocation();
		LoseSightTimer = LoseSightGrace;
	}
	else
	{
		LoseSightTimer -= DeltaSeconds;
		if (LoseSightTimer <= 0.f)
		{
			ChangeState(ETLSAIState::Search);
			return;
		}
	}

	Creature->GetCharacterMovement()->MaxWalkSpeed = Creature->HuntSpeed;

	RepathTimer -= DeltaSeconds;
	if (RepathTimer <= 0.f)
	{
		MoveToActor(Player, 90.f);
		RepathTimer = 0.4f;
	}

	Attack(Player, DeltaSeconds);
}

void ATLSCreatureController::Attack(ATLSCharacter* Player, float DeltaSeconds)
{
	const float Distance = FVector::Dist(Creature->GetActorLocation(), Player->GetActorLocation());
	if (Distance > Creature->AttackRange)
	{
		AttackTimer = 0.f;
		return;
	}

	// Lurkers only strike when the player is in the dark.
	if (Creature->CreatureType == ETLSCreatureType::Lurker && Player->Flashlight && Player->Flashlight->IsOn())
	{
		return;
	}

	AttackTimer -= DeltaSeconds;
	if (AttackTimer <= 0.f)
	{
		AttackTimer = Creature->AttackInterval;
		Creature->PlayAttack();
		Player->TakeDamage(Creature->AttackDamage, FDamageEvent(), this, Creature);
	}
}

void ATLSCreatureController::HandleTargetPerception(AActor* Actor, FAIStimulus Stimulus)
{
	ATLSCharacter* Player = Cast<ATLSCharacter>(Actor);
	if (!Player)
	{
		return;
	}

	if (Stimulus.Type == SightSenseID)
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			Target = Player;
			bSightContact = true;
			LastKnownLocation = Player->GetActorLocation();
			if (State != ETLSAIState::Retreat && State != ETLSAIState::Hunt)
			{
				ChangeState(ETLSAIState::Hunt);
			}
		}
		else
		{
			bSightContact = false;
		}
	}
	else if (Stimulus.Type == HearingSenseID && Stimulus.WasSuccessfullySensed())
	{
		// Sound only draws it over when it isn't already chasing or recovering.
		if (State == ETLSAIState::Dormant || State == ETLSAIState::Patrol)
		{
			NoiseLocation = Stimulus.StimulusLocation;
			ChangeState(ETLSAIState::Investigate);
		}
	}
}

bool ATLSCreatureController::IsPlayerLightingCreature(const ATLSCharacter* Player, const ATLSCreature* InCreature) const
{
	if (!Player->Flashlight || !Player->Flashlight->IsOn())
	{
		return false;
	}

	const FVector CamLoc = Player->FirstPersonCamera->GetComponentLocation();
	const FVector CamFwd = Player->FirstPersonCamera->GetForwardVector();
	const FVector Center = InCreature->GetActorLocation() + FVector(0.f, 0.f, 60.f);
	const FVector ToCreature = Center - CamLoc;

	if (ToCreature.Size() > FlashlightRange || FVector::DotProduct(CamFwd, ToCreature.GetSafeNormal()) < 0.8f)
	{
		return false; // Outside the beam's cone.
	}

	// Line of sight: the beam must reach the creature without hitting a wall.
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(TLSLightTrace), false);
	Params.AddIgnoredActor(Player);
	Params.AddIgnoredActor(InCreature);
	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit, CamLoc, Center, ECC_Visibility, Params);
	return !bBlocked;
}

bool ATLSCreatureController::MoveToRandomPointNear(const FVector& Center, float Radius, float Speed)
{
	FNavLocation Point;
	UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!Nav || !Nav->GetRandomReachablePointInRadius(GetWorld(), Center, Radius, Point))
	{
		return false;
	}
	Creature->GetCharacterMovement()->MaxWalkSpeed = Speed;
	MoveToLocation(Point.Location, 80.f);
	return true;
}

void ATLSCreatureController::ChangeState(ETLSAIState NewState)
{
	if (State == NewState && NewState != ETLSAIState::Patrol)
	{
		return;
	}

	State = NewState;
	bMoveIssued = false;
	RepathTimer = 0.f;
	AttackTimer = 0.f;
	StopMovement();

	switch (NewState)
	{
	case ETLSAIState::Dormant:
		StateTimer = 0.f;
		break;
	case ETLSAIState::Patrol:
		StateTimer = 0.f;
		break;
	case ETLSAIState::Investigate:
		StateTimer = 6.f;
		break;
	case ETLSAIState::Hunt:
		LoseSightTimer = LoseSightGrace;
		if (Creature)
		{
			Creature->PlayRandomVocal();
		}
		break;
	case ETLSAIState::Search:
		StateTimer = SearchDuration;
		break;
	case ETLSAIState::Retreat:
		StateTimer = Creature ? Creature->RetreatDuration : 6.f;
		break;
	}

	if (Creature)
	{
		Creature->OnBehaviourChanged(UEnum::GetValueAsString(NewState));
	}
}
