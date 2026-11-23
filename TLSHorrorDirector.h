#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TLSHorrorDirector.generated.h"

class ATLSCharacter;
class ATLSCreature;
class ATLSLevelDirector;
class ULightComponent;

/**
 * Raises and lowers tension from what is happening: nearby creatures, being hunted, low health and
 * darkness. Uses tension to drive flickering lights and ambient scares, so the horror lives
 * in pacing rather than constant jumpscares. Writes the value to the player for breathing audio and HUD.
 */
UCLASS()
class THELASTSIGNAL_API UTLSHorrorDirector : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	UFUNCTION(BlueprintPure, Category = "Horror")
	float GetTension() const { return Tension; }

	/** Plays a named scare from the level director. Use from trigger volumes or story beats. */
	UFUNCTION(BlueprintCallable, Category = "Horror")
	void RequestScare(FName ScareName);

	/** Forces a flicker wave now, regardless of tension. */
	UFUNCTION(BlueprintCallable, Category = "Horror")
	void FlickerLightsNow();

	/** Stops ambient scares and flicker, e.g. during a cutscene. */
	UFUNCTION(BlueprintCallable, Category = "Horror")
	void SetDirectorPaused(bool bPaused) { bPausedByScript = bPaused; }

private:
	struct FFlickeringLight
	{
		TWeakObjectPtr<ULightComponent> Light;
		float RemainingOff = 0.f;
	};

	void CollectLevelData(UWorld& World);
	void RefreshCreatures();
	float ComputeTargetTension(const ATLSCharacter* Player) const;
	void UpdateFlicker(float DeltaTime, bool bAllowed);
	void UpdateAmbientScares(float DeltaTime, const ATLSCharacter* Player, bool bAllowed);
	void StartFlickerWave();

	UPROPERTY()
	TObjectPtr<ATLSLevelDirector> LevelData;

	TArray<FFlickeringLight> FlickerLights;
	TArray<TWeakObjectPtr<ATLSCreature>> Creatures;

	float Tension = 0.f;
	float FlickerCooldown = 0.f;
	float AmbientCooldown = 0.f;
	float CreatureRefreshTimer = 0.f;
	bool bPausedByScript = false;
};
