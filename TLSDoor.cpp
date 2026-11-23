#include "Gameplay/TLSDoor.h"
#include "Player/TLSCharacter.h"
#include "Player/TLSInventoryComponent.h"
#include "Kismet/GameplayStatics.h"

ATLSDoor::ATLSDoor()
{
	PrimaryActorTick.bCanEverTick = true;
	InteractNoiseLoudness = 0.8f;
	Mesh->SetMobility(EComponentMobility::Movable);
}

void ATLSDoor::BeginPlay()
{
	Super::BeginPlay();
	ClosedYaw = GetActorRotation().Yaw;
	bIsLocked = !RequiredKeyId.IsNone();
}

void ATLSDoor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!FMath::IsNearlyEqual(CurrentYaw, TargetYaw, 0.1f))
	{
		CurrentYaw = FMath::FInterpConstantTo(CurrentYaw, TargetYaw, DeltaSeconds, OpenSpeed);
		FRotator R = GetActorRotation();
		R.Yaw = ClosedYaw + CurrentYaw;
		SetActorRotation(R);
	}
}

FText ATLSDoor::GetInteractPrompt(const ATLSCharacter* Interactor) const
{
	if (bIsLocked)
	{
		return FText::FromString(TEXT("Locked"));
	}
	return FText::FromString(bIsOpen ? TEXT("Close [E]") : TEXT("Open [E]"));
}

void ATLSDoor::OnInteract(ATLSCharacter* Interactor)
{
	if (!RequiredKeyId.IsNone() && !bUnlockedOnce)
	{
		UTLSInventoryComponent* Inv = Interactor ? Interactor->FindComponentByClass<UTLSInventoryComponent>() : nullptr;
		if (Inv && Inv->RemoveItem(RequiredKeyId, 1))
		{
			bUnlockedOnce = true;
			RequiredKeyId = NAME_None;
			bIsLocked = false;
		}
		else
		{
			bIsLocked = true;
			if (LockedSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, LockedSound, GetActorLocation());
			}
			ReportNoise(Interactor);
			return;
		}
	}

	bIsOpen = !bIsOpen;
	TargetYaw = bIsOpen ? OpenAngle : 0.f;
	if (OpenSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, OpenSound, GetActorLocation());
	}
	ReportNoise(Interactor);
}
