#include "Gameplay/TLSPickup.h"
#include "Player/TLSCharacter.h"
#include "Player/TLSInventoryComponent.h"
#include "Player/TLSFlashlightComponent.h"
#include "Kismet/GameplayStatics.h"

FText ATLSPickup::GetInteractPrompt(const ATLSCharacter* Interactor) const
{
	if (bIsBattery)
	{
		return FText::FromString(TEXT("Replace battery [E]"));
	}
	return FText::FromString(FString::Printf(TEXT("Take %s [E]"), *ItemId.ToString()));
}

void ATLSPickup::OnInteract(ATLSCharacter* Interactor)
{
	if (!Interactor)
	{
		return;
	}

	bool bTaken = false;
	if (bIsBattery)
	{
		bTaken = Interactor->Flashlight && Interactor->Flashlight->AddBattery(BatteryAmount);
	}
	else
	{
		bTaken = Interactor->Inventory && Interactor->Inventory->AddItem(ItemId, Count);
	}

	if (!bTaken)
	{
		return; // Full inventory or battery already charged: leave it in the world.
	}

	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
	}
	MarkConsumed();
	Destroy();
}
