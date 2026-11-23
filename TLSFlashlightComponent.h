#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TLSFlashlightComponent.generated.h"

class USpotLightComponent;
class USoundBase;

/**
 * Flashlight with a draining battery. Below LowBatteryThreshold the beam flickers.
 * Empty battery turns it off until a battery is inserted. Light also repels the creature
 * (see ATLSCreature::OnLightExposure).
 */
UCLASS(ClassGroup = (TLS), meta = (BlueprintSpawnableComponent))
class THELASTSIGNAL_API UTLSFlashlightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTLSFlashlightComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Hands the spot light to this component. The character calls this in BeginPlay. */
	void SetLightComponent(USpotLightComponent* InLight) { SpotLight = InLight; }

	UFUNCTION(BlueprintCallable, Category = "Flashlight")
	void Toggle();

	UFUNCTION(BlueprintCallable, Category = "Flashlight")
	void SetOn(bool bNewOn);

	/** Adds charge. Returns false if already full. */
	UFUNCTION(BlueprintCallable, Category = "Flashlight")
	bool AddBattery(float Amount = 50.f);

	UFUNCTION(BlueprintPure, Category = "Flashlight")
	bool IsOn() const { return bIsOn; }

	UFUNCTION(BlueprintPure, Category = "Flashlight")
	float GetBatteryPercent() const { return Battery / MaxBattery; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight", meta = (ClampMin = "0"))
	float MaxBattery = 100.f;

	/** Percent per second while on. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight")
	float DrainPerSecond = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight", meta = (ClampMin = "0", ClampMax = "1"))
	float LowBatteryThreshold = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight")
	float NormalIntensity = 6000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight")
	TObjectPtr<USoundBase> ClickSound;

	UPROPERTY(BlueprintReadOnly, Category = "Flashlight")
	float Battery = 100.f;

private:
	UPROPERTY()
	TObjectPtr<USpotLightComponent> SpotLight;

	bool bIsOn = false;
	float FlickerTimer = 0.f;
	void ApplyLightState();
};
