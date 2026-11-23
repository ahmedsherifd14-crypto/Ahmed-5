#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TLSMenuWidget.generated.h"

class UButton;
class USlider;
class UCheckBox;
class UTextBlock;

/**
 * Shared menu for the main menu and the pause menu. Any bound button that exists in the
 * Blueprint child is wired automatically; missing optional widgets are simply skipped.
 *
 * Expected widget names in the WBP_Menu child (case-sensitive):
 *   NewGameButton, ContinueButton, ResumeButton, SettingsButton, MainMenuButton, QuitButton, BackButton
 *   SettingsPanel (any panel), MasterVolumeSlider, QualitySlider, FullscreenCheck, VSyncCheck, QualityLabel
 *   MainPanel (any panel that holds the main buttons)
 */
UCLASS(Abstract)
class THELASTSIGNAL_API UTLSMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Level opened by New Game. Set per project in the Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	FString FirstLevelName = TEXT("Forest_01");

	/** True when this widget is shown in the pause menu rather than the main menu. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	bool bIsPauseMenu = false;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> NewGameButton;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> ContinueButton;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> ResumeButton;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> SettingsButton;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> MainMenuButton;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> QuitButton;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> BackButton;

	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UWidget> SettingsPanel;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UWidget> MainPanel;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<USlider> MasterVolumeSlider;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<USlider> QualitySlider;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UCheckBox> FullscreenCheck;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UCheckBox> VSyncCheck;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> QualityLabel;

private:
	UFUNCTION() void HandleNewGame();
	UFUNCTION() void HandleContinue();
	UFUNCTION() void HandleResume();
	UFUNCTION() void HandleSettings();
	UFUNCTION() void HandleBack();
	UFUNCTION() void HandleMainMenu();
	UFUNCTION() void HandleQuit();
	UFUNCTION() void HandleVolumeChanged(float Value);
	UFUNCTION() void HandleQualityChanged(float Value);
	UFUNCTION() void HandleFullscreenChanged(bool bChecked);
	UFUNCTION() void HandleVSyncChanged(bool bChecked);

	void ShowSettings(bool bShow);
	void RefreshLabels();
};
