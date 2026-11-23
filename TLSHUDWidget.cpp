#include "UI/TLSHUDWidget.h"
#include "Player/TLSCharacter.h"
#include "Player/TLSFlashlightComponent.h"
#include "Player/TLSPlayerController.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UTLSHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const ATLSCharacter* Player = GetOwningPlayerPawn<ATLSCharacter>();
	if (Player)
	{
		if (HealthBar)  HealthBar->SetPercent(Player->Health / Player->MaxHealth);
		if (StaminaBar) StaminaBar->SetPercent(Player->Stamina / Player->MaxStamina);
		if (BatteryBar && Player->Flashlight) BatteryBar->SetPercent(Player->Flashlight->GetBatteryPercent());
		if (TensionText)
		{
			TensionText->SetText(FText::AsPercent(Player->Tension));
		}

		if (PromptText)
		{
			const FText Prompt = Player->GetFocusPrompt();
			PromptText->SetText(Prompt);
			PromptText->SetVisibility(Prompt.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
		}
	}

	if (ObjectiveText)
	{
		if (const ATLSPlayerController* PC = GetOwningPlayer<ATLSPlayerController>())
		{
			ObjectiveText->SetText(PC->GetObjective());
		}
	}
}
