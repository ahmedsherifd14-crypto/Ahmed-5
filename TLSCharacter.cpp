#include "Player/TLSCharacter.h"
#include "Player/TLSFlashlightComponent.h"
#include "Player/TLSInventoryComponent.h"
#include "Gameplay/TLSInteractable.h"
#include "Components/SpotLightComponent.h"
#include "Components/AudioComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"

ATLSCharacter::ATLSCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->bOrientRotationToMovement = false;
	Move->MaxWalkSpeed = WalkSpeed;
	Move->MaxWalkSpeedCrouched = CrouchSpeed;
	Move->NavAgentProps.bCanCrouch = true;
	Move->bCanWalkOffLedgesWhenCrouching = false;

	GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->FieldOfView = 85.f;

	// The beam is parented to the camera so it always points where the player looks.
	FlashlightBeam = CreateDefaultSubobject<USpotLightComponent>(TEXT("FlashlightBeam"));
	FlashlightBeam->SetupAttachment(FirstPersonCamera);
	FlashlightBeam->SetIntensityUnits(ELightUnits::Candelas);
	FlashlightBeam->SetIntensity(6000.f);
	FlashlightBeam->SetInnerConeAngle(16.f);
	FlashlightBeam->SetOuterConeAngle(30.f);
	FlashlightBeam->SetAttenuationRadius(2500.f);
	FlashlightBeam->SetLightColor(FLinearColor(1.f, 0.96f, 0.88f));
	FlashlightBeam->SetCastShadows(true);
	FlashlightBeam->SetVisibility(false);

	Flashlight = CreateDefaultSubobject<UTLSFlashlightComponent>(TEXT("Flashlight"));
	Inventory = CreateDefaultSubobject<UTLSInventoryComponent>(TEXT("Inventory"));

	PerceptionSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PerceptionSource"));
	PerceptionSource->RegisterForSense(UAISense_Sight::StaticClass());
	PerceptionSource->RegisterForSense(UAISense_Hearing::StaticClass());
}

void ATLSCharacter::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	Stamina = MaxStamina;
	Flashlight->SetLightComponent(FlashlightBeam);
	PerceptionSource->RegisterWithPerceptionSystem();

	if (BreathingLoop)
	{
		BreathingComponent = UGameplayStatics::SpawnSoundAttached(BreathingLoop, GetRootComponent());
		if (BreathingComponent)
		{
			BreathingComponent->SetVolumeMultiplier(0.f);
		}
	}
}

void ATLSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ATLSCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ATLSCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ATLSCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ATLSCharacter::LookUp);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction(TEXT("Run"), IE_Pressed, this, &ATLSCharacter::StartRun);
	PlayerInputComponent->BindAction(TEXT("Run"), IE_Released, this, &ATLSCharacter::StopRun);
	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &ATLSCharacter::ToggleCrouch);
	PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &ATLSCharacter::OnInteractPressed);
	PlayerInputComponent->BindAction(TEXT("Flashlight"), IE_Pressed, this, &ATLSCharacter::OnFlashlightPressed);
}

void ATLSCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsDead())
	{
		return;
	}

	UpdateStamina(DeltaSeconds);
	UpdateSpeed();
	UpdateFocus();
	UpdateFootsteps(DeltaSeconds);
	UpdateTensionAudio(DeltaSeconds);
}

void ATLSCharacter::MoveForward(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ATLSCharacter::MoveRight(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ATLSCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ATLSCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ATLSCharacter::StartRun()
{
	bWantsRun = true;
}

void ATLSCharacter::StopRun()
{
	bWantsRun = false;
}

void ATLSCharacter::ToggleCrouch()
{
	if (IsCrouched())
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ATLSCharacter::OnFlashlightPressed()
{
	Flashlight->Toggle();
}

void ATLSCharacter::OnInteractPressed()
{
	if (ITLSInteractable* Target = Cast<ITLSInteractable>(FocusedActor.Get()))
	{
		Target->OnInteract(this);
	}
}

FText ATLSCharacter::GetFocusPrompt() const
{
	if (const ITLSInteractable* Target = Cast<ITLSInteractable>(FocusedActor.Get()))
	{
		return Target->GetInteractPrompt(this);
	}
	return FText::GetEmpty();
}

void ATLSCharacter::UpdateStamina(float DeltaSeconds)
{
	if (IsSprinting())
	{
		Stamina = FMath::Max(0.f, Stamina - StaminaDrainPerSecond * DeltaSeconds);
		if (Stamina <= 0.f)
		{
			bSprintLockedOut = true;
		}
	}
	else
	{
		Stamina = FMath::Min(MaxStamina, Stamina + StaminaRegenPerSecond * DeltaSeconds);
	}

	if (bSprintLockedOut && Stamina >= StaminaRecoverThreshold)
	{
		bSprintLockedOut = false;
	}
}

void ATLSCharacter::UpdateSpeed()
{
	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (IsCrouched())
	{
		Move->MaxWalkSpeedCrouched = CrouchSpeed;
	}
	else
	{
		Move->MaxWalkSpeed = IsSprinting() ? RunSpeed : WalkSpeed;
	}
}

void ATLSCharacter::UpdateFocus()
{
	FocusedActor = nullptr;

	const UCameraComponent* Cam = FirstPersonCamera;
	const FVector Start = Cam->GetComponentLocation();
	const FVector End = Start + Cam->GetForwardVector() * InteractDistance;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(TLSFocus), false, this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor->Implements<UTLSInteractable>())
		{
			FocusedActor = HitActor;
		}
	}
}

void ATLSCharacter::UpdateFootsteps(float DeltaSeconds)
{
	const bool bMoving = GetCharacterMovement()->IsMovingOnGround() && GetVelocity().SizeSquared2D() > 25.f;
	if (!bMoving)
	{
		DistanceSinceStep = 0.f;
		return;
	}

	DistanceSinceStep += GetVelocity().Size2D() * DeltaSeconds;

	float Threshold = IsSprinting() ? StepDistanceRun : StepDistanceWalk;
	if (IsCrouched())
	{
		Threshold *= 1.8f; // Crouched steps are quieter and further apart.
	}

	if (DistanceSinceStep >= Threshold)
	{
		DistanceSinceStep = 0.f;
		PlayFootstep();
	}
}

void ATLSCharacter::PlayFootstep()
{
	ETLSSurface Surface = ETLSSurface::Default;

	// Project Settings > Physics > Physical Surfaces: map SurfaceType1..7 to the ETLSSurface order.
	FHitResult Hit;
	const FVector Start = GetActorLocation();
	const FVector End = Start - FVector(0.f, 0.f, 160.f);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(TLSFootstep), false, this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params) && Hit.PhysMaterial.IsValid())
	{
		const int32 Index = static_cast<int32>(Hit.PhysMaterial->SurfaceType);
		if (Index >= 1 && Index <= 7)
		{
			Surface = static_cast<ETLSSurface>(Index);
		}
	}

	USoundBase* Sound = nullptr;
	if (const TObjectPtr<USoundBase>* Found = FootstepSounds.Find(Surface))
	{
		Sound = Found->Get();
	}
	else if (const TObjectPtr<USoundBase>* Fallback = FootstepSounds.Find(ETLSSurface::Default))
	{
		Sound = Fallback->Get();
	}

	if (Sound)
	{
		const float Volume = IsCrouched() ? 0.35f : (IsSprinting() ? 1.2f : 0.8f);
		UGameplayStatics::PlaySoundAtLocation(this, Sound, Start, Volume);
	}
}

void ATLSCharacter::UpdateTensionAudio(float DeltaSeconds)
{
	if (!BreathingComponent)
	{
		return;
	}

	// Breathing gets heavier when tired or when the director raises tension.
	const float Exhaustion = 1.f - (Stamina / MaxStamina);
	const float Target = FMath::Clamp(FMath::Max(Tension, Exhaustion * 0.6f) + (bHunted ? 0.25f : 0.f), 0.f, 1.f);
	BreathingVolume = FMath::FInterpTo(BreathingVolume, Target, DeltaSeconds, 2.f);
	BreathingComponent->SetVolumeMultiplier(BreathingVolume);
}

float ATLSCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (IsDead() || DamageAmount <= 0.f)
	{
		return 0.f;
	}

	Health = FMath::Max(0.f, Health - DamageAmount);
	OnHealthChanged.Broadcast(Health, MaxHealth);

	if (HurtSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HurtSound, GetActorLocation());
	}

	if (IsDead())
	{
		OnPlayerDied.Broadcast();
	}
	return DamageAmount;
}

void ATLSCharacter::Heal(float Amount)
{
	if (IsDead())
	{
		return;
	}
	Health = FMath::Min(MaxHealth, Health + Amount);
	OnHealthChanged.Broadcast(Health, MaxHealth);
}

void ATLSCharacter::SetHunted(bool bValue)
{
	bHunted = bValue;
}

void ATLSCharacter::RestoreVitals(float NewHealth, float NewStamina)
{
	Health = FMath::Clamp(NewHealth, 0.f, MaxHealth);
	Stamina = FMath::Clamp(NewStamina, 0.f, MaxStamina);
	OnHealthChanged.Broadcast(Health, MaxHealth);
}
