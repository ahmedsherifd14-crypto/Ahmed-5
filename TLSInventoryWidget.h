#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TLSInventoryWidget.generated.h"

class UTextBlock;

/** Inventory and journal screen. Refreshes itself whenever the inventory changes. */
UCLASS(Abstract)
class THELASTSIGNAL_API UTLSInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Display names for item IDs. Filled in the Blueprint; falls back to the raw ID. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TMap<FName, FText> ItemDisplayNames;

	/** Display titles for document IDs. Falls back to the raw ID. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TMap<FName, FText> DocumentTitles;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> ItemsText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> JournalText;

private:
	UFUNCTION() void Refresh();
};
