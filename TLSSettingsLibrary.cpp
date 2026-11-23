#include "UI/TLSSettingsLibrary.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/Engine.h"
#include "AudioDevice.h"
#include "Misc/ConfigCacheIni.h"

namespace
{
	const TCHAR* AudioSection = TEXT("TLSAudio");
	const TCHAR* MasterKey = TEXT("MasterVolume");
}

void UTLSSettingsLibrary::SetGraphicsQuality(int32 Level)
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	Settings->SetOverallScalabilityLevel(FMath::Clamp(Level, 0, 4));
	Settings->ApplySettings(false);
	Settings->SaveSettings();
}

int32 UTLSSettingsLibrary::GetGraphicsQuality()
{
	return UGameUserSettings::GetGameUserSettings()->GetOverallScalabilityLevel();
}

void UTLSSettingsLibrary::SetVSync(bool bEnabled)
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	Settings->SetVSyncEnabled(bEnabled);
	Settings->ApplySettings(false);
	Settings->SaveSettings();
}

void UTLSSettingsLibrary::SetFullscreen(bool bFullscreen)
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	Settings->SetFullscreenMode(bFullscreen ? EWindowMode::WindowedFullscreen : EWindowMode::Windowed);
	Settings->ApplySettings(false);
	Settings->SaveSettings();
}

void UTLSSettingsLibrary::SetMasterVolume(float Volume)
{
	const float Clamped = FMath::Clamp(Volume, 0.f, 1.f);
	if (GEngine)
	{
		if (FAudioDeviceHandle Device = GEngine->GetMainAudioDevice())
		{
			Device->SetTransientMasterVolume(Clamped);
		}
	}
	GConfig->SetFloat(AudioSection, MasterKey, Clamped, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
}

float UTLSSettingsLibrary::GetMasterVolume()
{
	float Value = 1.f;
	GConfig->GetFloat(AudioSection, MasterKey, Value, GGameUserSettingsIni);
	return FMath::Clamp(Value, 0.f, 1.f);
}

void UTLSSettingsLibrary::ApplySavedSettings()
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	Settings->ApplySettings(false);
	if (GEngine)
	{
		if (FAudioDeviceHandle Device = GEngine->GetMainAudioDevice())
		{
			Device->SetTransientMasterVolume(GetMasterVolume());
		}
	}
}
