#include "Core/TLSGameInstance.h"
#include "UI/TLSSettingsLibrary.h"

void UTLSGameInstance::Init()
{
	Super::Init();
	// Restore the player's saved graphics and audio choices before the first menu appears.
	UTLSSettingsLibrary::ApplySavedSettings();
}
