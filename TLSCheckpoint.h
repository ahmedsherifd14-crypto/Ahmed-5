#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TLSCheckpoint.generated.h"

class UBoxComponent;
class ATLSCharacter;

/** Walk through to save progress. Saves once per level visit; re-entering does not re-save. */
UCLASS()
class THELASTSIGNAL_API ATLSCheckpoint : public AActor
{
	GENERATED_BODY()

public:
	ATLSCheckpoint();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FName CheckpointId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	int32 ChapterIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> Trigger;

	/** Blueprint shows a "Progress saved" message here. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Checkpoint")
	void OnCheckpointSaved(ATLSCharacter* Player);

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	bool bSavedThisVisit = false;
};
