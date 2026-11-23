#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TLSPlayerController.generated.h"

class ATLSCharacter;
class UTLSMenuWidget;
class UTLSHUDWidget;
class UTLSInventoryWidget;
class UUserWidget;

/**
 * Owns UI flow: HUD, pause menu, inventory screen, objective text, main-menu mode,
 * and the death -> checkpoint loop.
 */
UCLASS()
class THELASTSIGNAL_API ATLSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATLSPlayerController();

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void SetupInputComponent() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ----- Widget classes (set in the controller Blueprint or the level) -----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UTLSHUDWidget> HUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UTLSMenuWidget> PauseMenuClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UTLSInventoryWidget> InventoryWidgetClass;

	/** Only used on the MainMenu map. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UTLSMenuWidget> MainMenuClass;

	/** Seconds between death and the checkpoint reload. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	float RespawnDelay = 2.5f;

	/** Chapter index stored with checkpoints. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	int32 ChapterIndex = 0;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetObjective(const FText& NewObjective);

	UFUNCTION(BlueprintPure, Category = "UI")
	FText GetObjective() const { return Objective; }

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleInventory();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void OpenPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ClosePauseMenu();

	UFUNCTION(BlueprintPure, Category = "UI")
	bool IsPauseOpen() const { return PauseWidget != nullptr; }

private:
	void TogglePause();

	UFUNCTION()
	void OnPlayerDied();
	void RespawnAtCheckpoint();
	void ShowGameInput();
	void ShowUIInput(UUserWidget* Focus);
	void SetupMainMenu();

	UPROPERTY()
	TObjectPtr<UTLSHUDWidget> HUDWidget;

	UPROPERTY()
	TObjectPtr<UTLSMenuWidget> PauseWidget;

	UPROPERTY()
	TObjectPtr<UTLSInventoryWidget> InventoryWidget;

	UPROPERTY()
	TObjectPtr<UTLSMenuWidget> MainMenuWidget;

	UPROPERTY()
	FText Objective;

	FTimerHandle RespawnTimer;
	TWeakObjectPtr<ATLSCharacter> BoundPlayer;
};
