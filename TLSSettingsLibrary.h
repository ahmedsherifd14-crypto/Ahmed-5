#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TLSSettingsLibrary.generated.h"

/** Graphics and audio settings. Values are saved to GameUserSettings and applied immediately. */
UCLASS()
class THELASTSIGNAL_API UTLSSettingsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** 0 = Low, 1 = Medium, 2 = High, 3 = Epic, 4 = Cinematic. Maps to engine scalability groups. */
	UFUNCTION(BlueprintCallable, Category = "TLS|Settings")
	static void SetGraphicsQuality(int32 Level);

	UFUNCTION(BlueprintPure, Category = "TLS|Settings")
	static int32 GetGraphicsQuality();

	UFUNCTION(BlueprintCallable, Category = "TLS|Settings")
	static void SetVSync(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "TLS|Settings")
	static void SetFullscreen(bool bFullscreen);

	/** 0..1. Applied to the main audio device and saved. */
	UFUNCTION(BlueprintCallable, Category = "TLS|Settings")
	static void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintPure, Category = "TLS|Settings")
	static float GetMasterVolume();

	/** Re-applies everything saved. Call from the game instance on startup. */
	UFUNCTION(BlueprintCallable, Category = "TLS|Settings")
	static void ApplySavedSettings();
};
