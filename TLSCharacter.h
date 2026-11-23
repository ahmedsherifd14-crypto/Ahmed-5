#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TLSCharacter.generated.h"

class UCameraComponent;
class UAudioComponent;
class USoundBase;
class UAIPerceptionStimuliSourceComponent;
class USpotLightComponent;
class UTLSFlashlightComponent;
class UTLSInventoryComponent;
class ATLSCreature;
class ATLSInteractableActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, Health, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);

UENUM(BlueprintType)
enum class ETLSSurface : uint8
{
	Default, Concrete, Metal, Wood, Water, Gravel, Carpet, Glass
};

/**
 * First-person survivor. Owns the flashlight, inventory, health, stamina and interaction.
 * Input uses legacy axis/action bindings (enable "Use Legacy Input" in Project Settings > Input).
 */
UCLASS()
class THELASTSIGNAL_API ATLSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATLSCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	// ----- Components -----
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpotLightComponent> FlashlightBeam;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTLSFlashlightComponent> Flashlight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTLSInventoryComponent> Inventory;

	/** Makes the player visible to creature sight and hearing. Without it the AI cannot sense the player. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAIPerceptionStimuliSourceComponent> PerceptionSource;

	// ----- Movement tuning -----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RunSpeed = 420.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float CrouchSpeed = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StepDistanceWalk = 160.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StepDistanceRun = 110.f;

	// ----- Health & stamina -----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals")
	float MaxHealth = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Vitals")
	float Health = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals")
	float MaxStamina = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Vitals")
	float Stamina = 100.f;

	/** Stamina drained per second while sprinting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals")
	float StaminaDrainPerSecond = 22.f;

	/** Stamina regenerated per second when not sprinting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals")
	float StaminaRegenPerSecond = 16.f;

	/** After being exhausted, sprint stays locked until stamina climbs back above this. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals")
	float StaminaRecoverThreshold = 25.f;

	/** Sprint is locked out until stamina recovers. */
	UPROPERTY(BlueprintReadOnly, Category = "Vitals")
	bool bSprintLockedOut = false;

	/** 0 = calm, 1 = terror. Written by the horror director; drives breathing and heartbeat. */
	UPROPERTY(BlueprintReadWrite, Category = "Vitals")
	float Tension = 0.f;

	// ----- Interaction -----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractDistance = 220.f;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<AActor> FocusedActor;

	// ----- Audio -----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TMap<ETLSSurface, TObjectPtr<USoundBase>> FootstepSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> BreathingLoop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> HurtSound;

	// ----- Events -----
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDied OnPlayerDied;

	// ----- API -----
	UFUNCTION(BlueprintCallable, Category = "Vitals")
	void Heal(float Amount);

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsSprinting() const { return bWantsRun && !bSprintLockedOut && Stamina > 0.f && GetVelocity().SizeSquared2D() > 1.f && !IsCrouched(); }

	UFUNCTION(BlueprintPure, Category = "Vitals")
	bool IsDead() const { return Health <= 0.f; }

	/** Prompt text for whatever the player is looking at, or empty. Used by the HUD. */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	FText GetFocusPrompt() const;

	/** Called by creature controllers via the perception system: this player is in a hunter's view. */
	UFUNCTION(BlueprintCallable, Category = "Vitals")
	void SetHunted(bool bValue);

	UFUNCTION(BlueprintPure, Category = "Vitals")
	bool IsHunted() const { return bHunted; }

	/** Re-apply saved vitals and transform. Used by the save subsystem. */
	void RestoreVitals(float NewHealth, float NewStamina);

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void StartRun();
	void StopRun();
	void ToggleCrouch();
	void OnInteractPressed();
	void OnFlashlightPressed();

	void UpdateStamina(float DeltaSeconds);
	void UpdateFocus();
	void UpdateFootsteps(float DeltaSeconds);
	void PlayFootstep();
	void UpdateSpeed();
	void UpdateTensionAudio(float DeltaSeconds);

	bool bWantsRun = false;
	bool bHunted = false;
	float DistanceSinceStep = 0.f;
	float BreathingVolume = 0.f;

	UPROPERTY()
	TObjectPtr<UAudioComponent> BreathingComponent;
};
