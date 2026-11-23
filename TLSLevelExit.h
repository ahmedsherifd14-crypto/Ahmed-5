#pragma once

#include "CoreMinimal.h"
#include "Gameplay/TLSInteractable.h"
#include "TLSLevelExit.generated.h"

/**
 * Moves the player to another level. Autosaves a checkpoint first, so a death after the
 * transition returns the player to the exit rather than to an earlier stage.
 * Refuses to open while any required flag is missing.
 */
UCLASS()
class THELASTSIGNAL_API ATLSLevelExit : public ATLSInteractableActor
{
	GENERATED_BODY()

public:
	ATLSLevelExit();

	virtual FText GetInteractPrompt(const ATLSCharacter* Interactor) const override;
	virtual void OnInteract(ATLSCharacter* Interactor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit")
	FName TargetLevel;

	/** Every flag must be set before the exit opens. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit")
	TArray<FName> RequiredFlags;

	/** Shown when a required flag is missing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit")
	FText LockedText;

	/** Chapter stored with the autosave. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit")
	int32 ChapterIndex = 0;

	/** Blueprint plays a transition effect here before the level opens. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Exit")
	void OnLevelExitUsed(ATLSCharacter* Interactor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Exit")
	void OnExitLocked();

private:
	bool AreRequirementsMet() const;
};
