#include "AI/TLSCreature.h"
#include "AI/TLSCreatureController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ATLSCreature::ATLSCreature()
{
	PrimaryActorTick.bCanEverTick = false;

	AIControllerClass = ATLSCreatureController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCapsuleComponent()->InitCapsuleSize(45.f, 96.f);
	GetCharacterMovement()->MaxWalkSpeed = PatrolWalkSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
}

void ATLSCreature::BeginPlay()
{
	Super::BeginPlay();

	// Per-type tuning, applied once so Blueprint overrides in the editor still win afterwards.
	switch (CreatureType)
	{
	case ETLSCreatureType::Stalker:
		SightRange = 2600.f;
		HearingRange = 1400.f;
		break;
	case ETLSCreatureType::Hunter:
		SightRange = 1400.f;
		HearingRange = 4200.f;
		HuntSpeed = 620.f;
		break;
	case ETLSCreatureType::Lurker:
		SightRange = 1800.f;
		HearingRange = 1600.f;
		AttackInterval = 0.9f;
		break;
	}
}

void ATLSCreature::PlayRandomVocal()
{
	if (Vocals.Num() == 0)
	{
		return;
	}
	const int32 Index = FMath::RandRange(0, Vocals.Num() - 1);
	if (Vocals[Index])
	{
		UGameplayStatics::PlaySoundAtLocation(this, Vocals[Index], GetActorLocation());
	}
}

void ATLSCreature::PlayRecoil()
{
	if (RecoilSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, RecoilSound, GetActorLocation());
	}
}

void ATLSCreature::PlayAttack()
{
	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
	}
}
