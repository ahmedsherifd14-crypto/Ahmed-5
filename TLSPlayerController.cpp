#include "Player/TLSPlayerController.h"
#include "Player/TLSCharacter.h"
#include "Player/TLSInventoryComponent.h"
#include "UI/TLSHUDWidget.h"
#include "UI/TLSMenuWidget.h"
#include "UI/TLSInventoryWidget.h"
#include "Save/TLSSaveSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ATLSPlayerController::ATLSPlayerController()
{
	bShowMouseCursor = false;
}

void ATLSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Tell the save system which level is live, so checkpoints record it.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTLSSaveSubsystem* Saves = GI->GetSubsystem<UTLSSaveSubsystem>())
		{
			Saves->SetCurrentLevelName(UWorld::RemovePIEPrefix(GetWorld()->GetMapName()));
		}
	}

	if (GetWorld()->GetMapName().Contains(TEXT("MainMenu")))
	{
		SetupMainMenu();
		return;
	}

	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UTLSHUDWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport(0);
		}
	}
	ShowGameInput();
}

void ATLSPlayerController::SetupMainMenu()
{
	if (!MainMenuClass)
	{
		return;
	}
	MainMenuWidget = CreateWidget<UTLSMenuWidget>(this, MainMenuClass);
	if (MainMenuWidget)
	{
		MainMenuWidget->AddToViewport(0);
		ShowUIInput(MainMenuWidget);
	}
}

void ATLSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ATLSCharacter* Player = Cast<ATLSCharacter>(InPawn);
	if (!Player)
	{
		return;
	}

	BoundPlayer = Player;
	Player->OnPlayerDied.AddUniqueDynamic(this, &ATLSPlayerController::OnPlayerDied);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTLSSaveSubsystem* Saves = GI->GetSubsystem<UTLSSaveSubsystem>())
		{
			Saves->ApplyPendingRestore(Player);
		}
	}
}

void ATLSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction(TEXT("Pause"), IE_Pressed, this, &ATLSPlayerController::TogglePause);
	InputComponent->BindAction(TEXT("Inventory"), IE_Pressed, this, &ATLSPlayerController::ToggleInventory);
}

void ATLSPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(RespawnTimer);
	Super::EndPlay(EndPlayReason);
}

void ATLSPlayerController::SetObjective(const FText& NewObjective)
{
	Objective = NewObjective;
}

void ATLSPlayerController::TogglePause()
{
	if (MainMenuWidget)
	{
		return; // Nothing to pause on the main menu.
	}
	if (PauseWidget)
	{
		ClosePauseMenu();
	}
	else
	{
		OpenPauseMenu();
	}
}

void ATLSPlayerController::OpenPauseMenu()
{
	if (PauseWidget || !PauseMenuClass)
	{
		return;
	}
	if (InventoryWidget)
	{
		ToggleInventory(); // Pause takes priority over the inventory screen.
	}
	PauseWidget = CreateWidget<UTLSMenuWidget>(this, PauseMenuClass);
	if (PauseWidget)
	{
		PauseWidget->bIsPauseMenu = true;
		PauseWidget->AddToViewport(100);
	}
	SetPause(true);
	ShowUIInput(PauseWidget);
}

void ATLSPlayerController::ClosePauseMenu()
{
	if (!PauseWidget)
	{
		return;
	}
	PauseWidget->RemoveFromParent();
	PauseWidget = nullptr;
	SetPause(false);
	ShowGameInput();
}

void ATLSPlayerController::ToggleInventory()
{
	if (PauseWidget)
	{
		return;
	}

	if (InventoryWidget)
	{
		InventoryWidget->RemoveFromParent();
		InventoryWidget = nullptr;
		ShowGameInput();
		return;
	}

	if (!InventoryWidgetClass)
	{
		return;
	}
	InventoryWidget = CreateWidget<UTLSInventoryWidget>(this, InventoryWidgetClass);
	if (InventoryWidget)
	{
		InventoryWidget->AddToViewport(50);
		ShowUIInput(InventoryWidget);
	}
}

void ATLSPlayerController::ShowGameInput()
{
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());
}

void ATLSPlayerController::ShowUIInput(UUserWidget* Focus)
{
	bShowMouseCursor = true;
	FInputModeUIOnly Mode;
	if (Focus)
	{
		Mode.SetWidgetToFocus(Focus->TakeWidget());
	}
	SetInputMode(Mode);
}

void ATLSPlayerController::OnPlayerDied()
{
	if (APlayerCameraManager* Cam = PlayerCameraManager)
	{
		Cam->StartCameraFade(0.f, 1.f, 1.2f, FLinearColor::Black, false, true);
	}
	GetWorldTimerManager().SetTimer(RespawnTimer, this, &ATLSPlayerController::RespawnAtCheckpoint,
		RespawnDelay, false);
}

void ATLSPlayerController::RespawnAtCheckpoint()
{
	UGameInstance* GI = GetGameInstance();
	UTLSSaveSubsystem* Saves = GI ? GI->GetSubsystem<UTLSSaveSubsystem>() : nullptr;
	if (Saves && Saves->LoadCheckpoint())
	{
		return; // Level change in progress; the restore happens on the next OnPossess.
	}

	// No checkpoint yet: restart the current level from scratch.
	UGameplayStatics::OpenLevel(this, FName(*UWorld::RemovePIEPrefix(GetWorld()->GetMapName())));
}
