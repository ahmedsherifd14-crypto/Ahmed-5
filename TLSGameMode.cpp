#include "Core/TLSGameMode.h"
#include "Player/TLSCharacter.h"
#include "Player/TLSPlayerController.h"

ATLSGameMode::ATLSGameMode()
{
	DefaultPawnClass = ATLSCharacter::StaticClass();
	PlayerControllerClass = ATLSPlayerController::StaticClass();
	bUseSeamlessTravel = false;
}

void ATLSGameMode::StartPlay()
{
	Super::StartPlay();
}
