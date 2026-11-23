#pragma once

#include "CoreMinimal.h"
#include "Gameplay/TLSInteractable.h"
#include "TLSDoor.generated.h"

class USoundBase;

/** Hinged door. Optionally locked behind a key item. Slams and makes noise. */
UCLASS()
class THELASTSIGNAL_API ATLSDoor : public ATLSInteractableActor
{
	GENERATED_BODY()

public:
	ATLSDoor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual FText GetInteractPrompt(const ATLSCharacter* Interactor) const override;
	virtual void OnInteract(ATLSCharacter* Interactor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	float OpenAngle = 95.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	float OpenSpeed = 90.f;

	/** Empty = unlocked. Otherwise the inventory item ID that unlocks this door (consumed). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	FName RequiredKeyId;

	UPROPERTY(BlueprintReadOnly, Category = "Door")
	bool bIsOpen = false;

	UPROPERTY(BlueprintReadOnly, Category = "Door")
	bool bIsLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	TObjectPtr<USoundBase> OpenSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	TObjectPtr<USoundBase> LockedSound;

private:
	float ClosedYaw = 0.f;
	float CurrentYaw = 0.f;
	float TargetYaw = 0.f;
	bool bUnlockedOnce = false;
};
