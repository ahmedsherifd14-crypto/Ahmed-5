#include "Horror/TLSHorrorDirector.h"
#include "Horror/TLSLevelDirector.h"
#include "AI/TLSCreature.h"
#include "Player/TLSCharacter.h"
#include "Player/TLSFlashlightComponent.h"
#include "Components/LightComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void UTLSHorrorDirector::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UTLSHorrorDirector::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	CollectLevelData(InWorld);
	Tension = 0.f;
	FlickerCooldown = FMath::RandRange(8.f, 14.f);
	AmbientCooldown = FMath::RandRange(15.f, 25.f);
}

TStatId UTLSHorrorDirector::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UTLSHorrorDirector, STATGROUP_Tickables);
}

void UTLSHorrorDirector::CollectLevelData(UWorld& World)
{
	LevelData = nullptr;
	for (TActorIterator<ATLSLevelDirector> It(&World); It; ++It)
	{
		LevelData = *It;
		break;
	}

	FlickerLights.Reset();
	for (TActorIterator<AActor> It(&World); It; ++It)
	{
		if (It->ActorHasTag(TEXT("TLS_Flicker")))
		{
			if (ULightComponent* Light = It->FindComponentByClass<ULightComponent>())
			{
				FFlickeringLight Entry;
				Entry.Light = Light;
				FlickerLights.Add(Entry);
			}
		}
	}
}

void UTLSHorrorDirector::RefreshCreatures()
{
	Creatures.Reset();
	if (!GetWorld())
	{
		return;
	}
	for (TActorIterator<ATLSCreature> It(GetWorld()); It; ++It)
	{
		Creatures.Add(*It);
	}
}

float UTLSHorrorDirector::ComputeTargetTension(const ATLSCharacter* Player) const
{
	float Target = 0.f;

	// Nearest creature drives most of the tension.
	float Nearest = TNumericLimits<float>::Max();
	for (const TWeakObjectPtr<ATLSCreature>& C : Creatures)
	{
		if (C.IsValid())
		{
			Nearest = FMath::Min(Nearest, FVector::Dist(C->GetActorLocation(), Player->GetActorLocation()));
		}
	}
	if (Nearest < 2500.f)
	{
		Target += FMath::GetMappedRangeValueClamped(FVector2D(2500.f, 500.f), FVector2D(0.f, 0.7f), Nearest);
	}

	if (Player->IsHunted())
	{
		Target += 0.35f;
	}

	// Low health and a dying flashlight both make the player feel exposed.
	Target += (1.f - Player->Health / Player->MaxHealth) * 0.2f;
	if (Player->Flashlight && !Player->Flashlight->IsOn())
	{
		Target += 0.1f;
	}

	return FMath::Clamp(Target, 0.f, 1.f);
}

void UTLSHorrorDirector::Tick(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ATLSCharacter* Player = Cast<ATLSCharacter>(UGameplayStatics::GetPlayerPawn(World, 0));
	if (!Player || Player->IsDead())
	{
		return;
	}

	CreatureRefreshTimer -= DeltaTime;
	if (CreatureRefreshTimer <= 0.f)
	{
		RefreshCreatures();
		CreatureRefreshTimer = 2.f;
	}

	const float Scale = LevelData ? LevelData->TensionScale : 1.f;
	const float Target = ComputeTargetTension(Player);

	// Tension rises quickly when threatened and fades slowly once it's safe.
	const float Rate = (Target > Tension) ? 0.9f : 0.25f;
	Tension = FMath::FInterpTo(Tension, Target, DeltaTime, Rate * Scale);
	Player->Tension = Tension;

	const bool bAllowed = !bPausedByScript && !Player->IsHunted();
	UpdateFlicker(DeltaTime, bAllowed);
	UpdateAmbientScares(DeltaTime, Player, bAllowed);
}

void UTLSHorrorDirector::UpdateFlicker(float DeltaTime, bool bAllowed)
{
	const float Threshold = LevelData ? LevelData->FlickerThreshold : 0.3f;

	FlickerCooldown -= DeltaTime;
	if (bAllowed && Tension >= Threshold && FlickerCooldown <= 0.f && FlickerLights.Num() > 0)
	{
		StartFlickerWave();
		// Higher tension means more frequent flicker.
		FlickerCooldown = FMath::Lerp(14.f, 5.f, Tension) * FMath::RandRange(0.8f, 1.2f);
	}

	for (int32 i = FlickerLights.Num() - 1; i >= 0; --i)
	{
		FFlickeringLight& F = FlickerLights[i];
		if (!F.Light.IsValid())
		{
			FlickerLights.RemoveAtSwap(i);
			continue;
		}
		if (F.RemainingOff > 0.f)
		{
			F.RemainingOff -= DeltaTime;
			if (F.RemainingOff <= 0.f)
			{
				F.Light->SetVisibility(true);
			}
		}
	}
}

void UTLSHorrorDirector::StartFlickerWave()
{
	// Flicker a random subset so the whole room doesn't blink in sync.
	for (FFlickeringLight& F : FlickerLights)
	{
		if (F.Light.IsValid() && FMath::FRand() < 0.6f)
		{
			F.Light->SetVisibility(false);
			F.RemainingOff = FMath::RandRange(0.12f, 0.7f);
		}
	}
}

void UTLSHorrorDirector::FlickerLightsNow()
{
	if (FlickerLights.Num() > 0)
	{
		StartFlickerWave();
	}
}

void UTLSHorrorDirector::UpdateAmbientScares(float DeltaTime, const ATLSCharacter* Player, bool bAllowed)
{
	AmbientCooldown -= DeltaTime;
	if (!bAllowed || AmbientCooldown > 0.f || !LevelData || LevelData->AmbientScares.Num() == 0)
	{
		return;
	}

	// Sounds only at moderate tension: calm areas stay calm, and high tension is already loud enough.
	if (Tension < 0.15f || Tension > 0.75f)
	{
		AmbientCooldown = 2.f;
		return;
	}

	const int32 Index = FMath::RandRange(0, LevelData->AmbientScares.Num() - 1);
	USoundBase* Sound = LevelData->AmbientScares[Index].Get();
	if (Sound)
	{
		// Place the sound behind or beside the player so it can't be seen.
		const float Angle = FMath::RandRange(120.f, 240.f);
		const FRotator Yaw(0.f, Player->GetControlRotation().Yaw + Angle, 0.f);
		const FVector Pos = Player->GetActorLocation() + Yaw.Vector() * FMath::RandRange(900.f, 1600.f);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Pos, FMath::RandRange(0.6f, 1.0f));
	}

	AmbientCooldown = FMath::RandRange(LevelData->AmbientScareInterval.X, LevelData->AmbientScareInterval.Y);
}

void UTLSHorrorDirector::RequestScare(FName ScareName)
{
	if (!LevelData)
	{
		return;
	}
	if (const TObjectPtr<USoundBase>* Found = LevelData->NamedScares.Find(ScareName))
	{
		if (USoundBase* Sound = Found->Get())
		{
			ATLSCharacter* Player = Cast<ATLSCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
			const FVector Where = Player ? Player->GetActorLocation() : FVector::ZeroVector;
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Where, 1.f);
		}
	}
}
