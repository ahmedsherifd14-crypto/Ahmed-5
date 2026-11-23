#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Player/TLSInventoryComponent.h"
#include "TLSSaveGame.generated.h"

/** Everything needed to resume at a checkpoint. Story flags carry pickups, puzzles and decisions. */
UCLASS()
class THELASTSIGNAL_API UTLSSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere) FString LevelName;
	UPROPERTY(VisibleAnywhere) FName CheckpointId;
	UPROPERTY(VisibleAnywhere) FVector PlayerLocation = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere) FRotator PlayerRotation = FRotator::ZeroRotator;
	UPROPERTY(VisibleAnywhere) float Health = 100.f;
	UPROPERTY(VisibleAnywhere) float Stamina = 100.f;
	UPROPERTY(VisibleAnywhere) float Battery = 100.f;
	UPROPERTY(VisibleAnywhere) TArray<FTLSInventoryEntry> Items;
	UPROPERTY(VisibleAnywhere) TArray<FName> Documents;
	UPROPERTY(VisibleAnywhere) TArray<FName> Flags;
	UPROPERTY(VisibleAnywhere) int32 ChapterIndex = 0;
	UPROPERTY(VisibleAnywhere) FString SavedAt;
};
