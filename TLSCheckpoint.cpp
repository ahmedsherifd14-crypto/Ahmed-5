#include "Gameplay/TLSCheckpoint.h"
#include "Components/BoxComponent.h"
#include "Player/TLSCharacter.h"
#include "Save/TLSSaveSubsystem.h"
#include "Engine/GameInstance.h"

ATLSCheckpoint::ATLSCheckpoint()
{
	PrimaryActorTick.bCanEverTick = false;
	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	SetRootComponent(Trigger);
	Trigger->SetBoxExtent(FVector(120.f, 120.f, 120.f));
	Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ATLSCheckpoint::BeginPlay()
{
	Super::BeginPlay();
	Trigger->OnComponentBeginOverlap.AddUniqueDynamic(this, &ATLSCheckpoint::HandleBeginOverlap);
}

void ATLSCheckpoint::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ATLSCharacter* Player = Cast<ATLSCharacter>(OtherActor);
	if (!Player || bSavedThisVisit)
	{
		return;
	}

	if (UTLSSaveSubsystem* Saves = GetGameInstance()->GetSubsystem<UTLSSaveSubsystem>())
	{
		if (Saves->SaveCheckpoint(Player, CheckpointId, ChapterIndex))
		{
			bSavedThisVisit = true;
			OnCheckpointSaved(Player);
		}
	}
}
