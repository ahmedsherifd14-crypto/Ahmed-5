#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TLSHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * In-game HUD. Polls the owning character and controller each frame, so it never goes stale.
 * Optional bound widgets: HealthBar, StaminaBar, BatteryBar, PromptText, ObjectiveText, TensionText.
 */
UCLASS(Abstract)
class THELASTSIGNAL_API UTLSHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UProgressBar> HealthBar;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UProgressBar> StaminaBar;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UProgressBar> BatteryBar;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> PromptText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> ObjectiveText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> TensionText;
};
