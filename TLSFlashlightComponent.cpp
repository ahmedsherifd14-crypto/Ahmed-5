#include "Player/TLSFlashlightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Kismet/GameplayStatics.h"

UTLSFlashlightComponent::UTLSFlashlightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.f / 30.f;
}

void UTLSFlashlightComponent::BeginPlay()
{
	Super::BeginPlay();
	Battery = MaxBattery;
	ApplyLightState();
}

void UTLSFlashlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsOn)
	{
		return;
	}

	Battery = FMath::Max(0.f, Battery - DrainPerSecond * DeltaTime * (MaxBattery / 100.f));

	if (Battery <= 0.f)
	{
		SetOn(false);
		return;
	}

	if (SpotLight)
	{
		const bool bLow = GetBatteryPercent() < LowBatteryThreshold;
		if (bLow)
		{
			// Irregular flicker that gets worse as the battery approaches zero.
			FlickerTimer -= DeltaTime;
			if (FlickerTimer <= 0.f)
			{
				const float Severity = 1.f - (GetBatteryPercent() / LowBatteryThreshold);
				FlickerTimer = FMath::RandRange(0.03f, 0.25f) * (1.f - Severity * 0.6f);
				const bool bDip = FMath::FRand() < (0.3f + Severity * 0.5f);
				SpotLight->SetIntensity(bDip ? 0.f : NormalIntensity * FMath::FRandRange(0.4f, 1.f));
			}
		}
		else
		{
			SpotLight->SetIntensity(NormalIntensity);
		}
	}
}

void UTLSFlashlightComponent::Toggle()
{
	SetOn(!bIsOn);
}

void UTLSFlashlightComponent::SetOn(bool bNewOn)
{
	if (bNewOn && Battery <= 0.f)
	{
		return; // Dead battery: nothing happens.
	}
	if (bNewOn == bIsOn)
	{
		return;
	}
	bIsOn = bNewOn;
	if (ClickSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ClickSound, GetOwner()->GetActorLocation());
	}
	ApplyLightState();
}

bool UTLSFlashlightComponent::AddBattery(float Amount)
{
	if (Battery >= MaxBattery)
	{
		return false;
	}
	Battery = FMath::Min(MaxBattery, Battery + Amount);
	return true;
}

void UTLSFlashlightComponent::ApplyLightState()
{
	if (SpotLight)
	{
		SpotLight->SetVisibility(bIsOn);
		SpotLight->SetIntensity(bIsOn ? NormalIntensity : 0.f);
	}
}
