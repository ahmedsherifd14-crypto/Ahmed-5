#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TLSSaveSubsystem.generated.h"

class ATLSCharacter;
class UTLSSaveGame;

/**
 * Owns checkpoint saving and loading across level transitions.
 * Loading a checkpoint stores a pending restore; the player controller applies it once the
 * target level has spawned the character.
 */
UCLASS()
class THELASTSIGNAL_API UTLSSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static constexpr const TCHAR* AutoSlot = TEXT("TLS_Checkpoint");

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool HasSave() const;

	/** Writes the player's state and the current level as the latest checkpoint. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveCheckpoint(ATLSCharacter* Player, FName CheckpointId, int32 ChapterIndex);

	/** Opens the saved level and queues the restore. Returns false if no save exists. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadCheckpoint();

	/** Starts a fresh campaign: clears flags and the checkpoint, then opens the first level. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void StartNewGame(const FString& FirstLevel);

	/** Called by the controller after possessing a pawn in a level. */
	bool ApplyPendingRestore(ATLSCharacter* Player);

	bool HasPendingRestore() const { return PendingSave != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Save")
	FString GetCurrentLevelName() const { return CurrentLevel; }

	void SetCurrentLevelName(const FString& LevelName) { CurrentLevel = LevelName; }

private:
	UPROPERTY()
	TObjectPtr<UTLSSaveGame> PendingSave;

	FString CurrentLevel;
};
