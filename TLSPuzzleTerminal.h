#pragma once

#include "CoreMinimal.h"
#include "Gameplay/TLSInteractable.h"
#include "TLSPuzzleTerminal.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleSolved);

/**
 * Keypad-style puzzle. The Blueprint opens the keypad UI through OnOpenKeypad; the widget calls
 * SubmitCode. Solving it sets SolvedFlag and optionally needs RequiredFlag first (e.g. power on).
 */
UCLASS()
class THELASTSIGNAL_API ATLSPuzzleTerminal : public ATLSInteractableActor
{
	GENERATED_BODY()

public:
	ATLSPuzzleTerminal();

	virtual FText GetInteractPrompt(const ATLSCharacter* Interactor) const override;
	virtual void OnInteract(ATLSCharacter* Interactor) override;

	/** Returns true when the code is correct and the puzzle is now solved. */
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	bool SubmitCode(const FString& Code);

	UFUNCTION(BlueprintPure, Category = "Puzzle")
	bool IsSolved() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
	FString CorrectCode = TEXT("4071");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
	FName SolvedFlag;

	/** Flag that must already be set before the keypad accepts input. Empty = none. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
	FName RequiredFlag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
	FText LockedMessage;

	/** Blueprint shows the keypad widget here. */
	UPROPERTY(BlueprintAssignable, Category = "Puzzle")
	FOnPuzzleSolved OnPuzzleSolved;

	UFUNCTION(BlueprintImplementableEvent, Category = "Puzzle")
	void OnOpenKeypad(ATLSCharacter* Interactor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Puzzle")
	void OnRejected();
};
