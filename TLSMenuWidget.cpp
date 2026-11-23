#include "UI/TLSMenuWidget.h"
#include "UI/TLSSettingsLibrary.h"
#include "Engine/GameInstance.h"
#include "Save/TLSSaveSubsystem.h"
#include "Player/TLSPlayerController.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
	const TCHAR* QualityNames[] = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic"), TEXT("Cinematic") };
}

void UTLSMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (NewGameButton)   NewGameButton->OnClicked.AddDynamic(this, &UTLSMenuWidget::HandleNewGame);
	if (ContinueButton)  ContinueButton->OnClicked.AddDynamic(this, &UTLSMenuWidget::HandleContinue);
	if (ResumeButton)    ResumeButton->OnClicked.AddDynamic(this, &UTLSMenuWidget::HandleResume);
	if (SettingsButton)  SettingsButton->OnClicked.AddDynamic(this, &UTLSMenuWidget::HandleSettings);
	if (BackButton)      BackButton->OnClicked.AddDynamic(this, &UTLSMenuWidget::HandleBack);
	if (MainMenuButton)  MainMenuButton->OnClicked.AddDynamic(this, &UTLSMenuWidget::HandleMainMenu);
	if (QuitButton)      QuitButton->OnClicked.AddDynamic(this, &UTLSMenuWidget::HandleQuit);

	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->SetValue(UTLSSettingsLibrary::GetMasterVolume());
		MasterVolumeSlider->OnValueChanged.AddDynamic(this, &UTLSMenuWidget::HandleVolumeChanged);
	}
	if (QualitySlider)
	{
		QualitySlider->SetMinValue(0.f);
		QualitySlider->SetMaxValue(4.f);
		QualitySlider->SetStepSize(1.f);
		QualitySlider->SetValue(static_cast<float>(UTLSSettingsLibrary::GetGraphicsQuality()));
		QualitySlider->OnValueChanged.AddDynamic(this, &UTLSMenuWidget::HandleQualityChanged);
	}
	if (FullscreenCheck)
	{
		const UGameUserSettings* S = UGameUserSettings::GetGameUserSettings();
		FullscreenCheck->SetIsChecked(S->GetFullscreenMode() != EWindowMode::Windowed);
		FullscreenCheck->OnCheckStateChanged.AddDynamic(this, &UTLSMenuWidget::HandleFullscreenChanged);
	}
	if (VSyncCheck)
	{
		VSyncCheck->SetIsChecked(UGameUserSettings::GetGameUserSettings()->IsVSyncEnabled());
		VSyncCheck->OnCheckStateChanged.AddDynamic(this, &UTLSMenuWidget::HandleVSyncChanged);
	}

	// Main menu: Continue only if a save exists. Pause menu: no New Game, no Continue.
	UTLSSaveSubsystem* Saves = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTLSSaveSubsystem>() : nullptr;
	if (ContinueButton)
	{
		ContinueButton->SetIsEnabled(Saves && Saves->HasSave());
	}
	if (bIsPauseMenu)
	{
		if (NewGameButton)  NewGameButton->SetVisibility(ESlateVisibility::Collapsed);
		if (ContinueButton) ContinueButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	else if (ResumeButton)
	{
		ResumeButton->SetVisibility(ESlateVisibility::Collapsed);
		if (MainMenuButton) MainMenuButton->SetVisibility(ESlateVisibility::Collapsed);
	}

	ShowSettings(false);
	RefreshLabels();
}

void UTLSMenuWidget::HandleNewGame()
{
	if (UTLSSaveSubsystem* Saves = GetGameInstance()->GetSubsystem<UTLSSaveSubsystem>())
	{
		Saves->StartNewGame(FirstLevelName);
	}
}

void UTLSMenuWidget::HandleContinue()
{
	if (UTLSSaveSubsystem* Saves = GetGameInstance()->GetSubsystem<UTLSSaveSubsystem>())
	{
		Saves->LoadCheckpoint();
	}
}

void UTLSMenuWidget::HandleResume()
{
	if (ATLSPlayerController* PC = Cast<ATLSPlayerController>(GetOwningPlayer()))
	{
		PC->ClosePauseMenu();
	}
}

void UTLSMenuWidget::HandleSettings()
{
	ShowSettings(true);
}

void UTLSMenuWidget::HandleBack()
{
	ShowSettings(false);
}

void UTLSMenuWidget::HandleMainMenu()
{
	UGameplayStatics::OpenLevel(this, FName(TEXT("MainMenu")));
}

void UTLSMenuWidget::HandleQuit()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UTLSMenuWidget::HandleVolumeChanged(float Value)
{
	UTLSSettingsLibrary::SetMasterVolume(Value);
}

void UTLSMenuWidget::HandleQualityChanged(float Value)
{
	UTLSSettingsLibrary::SetGraphicsQuality(FMath::RoundToInt(Value));
	RefreshLabels();
}

void UTLSMenuWidget::HandleFullscreenChanged(bool bChecked)
{
	UTLSSettingsLibrary::SetFullscreen(bChecked);
}

void UTLSMenuWidget::HandleVSyncChanged(bool bChecked)
{
	UTLSSettingsLibrary::SetVSync(bChecked);
}

void UTLSMenuWidget::ShowSettings(bool bShow)
{
	if (SettingsPanel)
	{
		SettingsPanel->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	if (MainPanel)
	{
		MainPanel->SetVisibility(bShow ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}

void UTLSMenuWidget::RefreshLabels()
{
	if (QualityLabel)
	{
		const int32 Level = FMath::Clamp(UTLSSettingsLibrary::GetGraphicsQuality(), 0, 4);
		QualityLabel->SetText(FText::FromString(QualityNames[Level]));
	}
}
