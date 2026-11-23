#pragma once

#include "CoreMinimal.h"
#include "Gameplay/TLSInteractable.h"
#include "Core/TLSGameInstance.h"
#include "TLSEndingTerminal.generated.h"

USTRUCT(BlueprintType)
struct FTLSEndingOption
{
	GENERATED_BODY()

	/** Which ending this option leads to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) ETLSEnding Ending = ETLSEnding::None;

	/** Level that plays the ending cinematic and credits. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName LevelName;

	/** Every flag here must be set for the option to be offered. Empty = always offered. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> RequiredFlags;

	/** Label the Blueprint shows on the choice button. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Label;
};

/**
 * Final-chapter terminal. Offers the endings whose required flags the player has earned, then
 * records the choice and loads that ending's level.
 */
UCLASS()
class THELASTSIGNAL_API ATLSEndingTerminal : public ATLSInteractableActor
{
	GENERATED_BODY()

public:
	ATLSEndingTerminal();

	virtual FText GetInteractPrompt(const ATLSCharacter* Interactor) const override;
	virtual void OnInteract(ATLSCharacter* Interactor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ending")
	TArray<FTLSEndingOption> Options;

	/** Returns the options the player currently qualifies for. */
	UFUNCTION(BlueprintPure, Category = "Ending")
	TArray<FTLSEndingOption> GetAvailableOptions() const;

	/** Records the ending and opens its level. Returns false if the option is not available. */
	UFUNCTION(BlueprintCallable, Category = "Ending")
	bool ChooseEnding(ETLSEnding Ending);

	/** Blueprint builds and shows the choice UI from GetAvailableOptions. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ending")
	void OnShowChoices(ATLSCharacter* Interactor);
};
