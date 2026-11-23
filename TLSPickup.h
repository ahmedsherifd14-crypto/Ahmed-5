#pragma once

#include "CoreMinimal.h"
#include "Gameplay/TLSInteractable.h"
#include "TLSPickup.generated.h"

class USoundBase;

/** Item pickup (battery, medkit, key). Removed from the world once taken. */
UCLASS()
class THELASTSIGNAL_API ATLSPickup : public ATLSInteractableActor
{
	GENERATED_BODY()

public:
	virtual FText GetInteractPrompt(const ATLSCharacter* Interactor) const override;
	virtual void OnInteract(ATLSCharacter* Interactor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	FName ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", meta = (ClampMin = "1"))
	int32 Count = 1;

	/** Battery pickups recharge the flashlight instead of entering the inventory. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	bool bIsBattery = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float BatteryAmount = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	TObjectPtr<USoundBase> PickupSound;
};
