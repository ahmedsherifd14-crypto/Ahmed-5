#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TLSGameInstance.generated.h"

/** Ending the player reached. Set by story triggers, read by the final chapter. */
UENUM(BlueprintType)
enum class ETLSEnding : uint8
{
	None,
	Escape,        // Signal traced, player leaves through the forest
	Transmit,      // Player broadcasts the signal back into the facility
	Stay           // Player stays with the entity, accepting its offer
};

/** Persistent state across levels and save slots. */
UCLASS()
class THELASTSIGNAL_API UTLSGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UPROPERTY(BlueprintReadWrite, Category = "Story")
	ETLSEnding ChosenEnding = ETLSEnding::None;

	/** Flags for story decisions (e.g. "SpareSurvivor", "ReadLogA"). */
	UPROPERTY(BlueprintReadWrite, Category = "Story")
	TSet<FName> StoryFlags;

	UFUNCTION(BlueprintCallable, Category = "Story")
	void SetFlag(FName Flag) { StoryFlags.Add(Flag); }

	UFUNCTION(BlueprintPure, Category = "Story")
	bool HasFlag(FName Flag) const { return StoryFlags.Contains(Flag); }
};
