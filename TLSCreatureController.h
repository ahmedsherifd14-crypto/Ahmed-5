#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "TLSCreatureController.generated.h"

class UAIPerceptionComponent;
class ATLSCharacter;
class ATLSCreature;

UENUM()
enum class ETLSAIState : uint8
{
	Dormant,      // Idle, waiting to be woken by a sound or sight
	Patrol,       // Roams around its home point
	Investigate,  // Walks to a heard noise, then looks around
	Hunt,         // Has the player in sight (or recently had) and chases
	Search,       // Lost the player: sweeps around the last known position
	Retreat       // Recoiled from the flashlight, backs off for a while
};

/**
 * Drives the creature. Perception feeds the state machine: sight starts a hunt, sound starts an
 * investigation, the flashlight causes a recoil (Stalker and Lurker), and the Lurker only attacks
 * in darkness. Movement uses the navmesh.
 */
UCLASS()
class THELASTSIGNAL_API ATLSCreatureController : public AAIController
{
	GENERATED_BODY()

public:
	ATLSCreatureController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;

	ETLSAIState GetAIState() const { return State; }

	/** Seconds to hold the chase after losing sight before giving up. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float LoseSightGrace = 4.f;

	/** Seconds the creature sweeps after losing the player. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float SearchDuration = 14.f;

	/** Distance from the flashlight beam's end to the creature still counted as lit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float FlashlightRange = 2000.f;

private:
	UFUNCTION()
	void HandleTargetPerception(AActor* Actor, FAIStimulus Stimulus);

	void ConfigurePerception(ATLSCreature* Creature);
	void ChangeState(ETLSAIState NewState);
	void TickState(float DeltaSeconds);
	void TickHunt(float DeltaSeconds);
	bool IsPlayerLightingCreature(const ATLSCharacter* Player, const ATLSCreature* Creature) const;
	bool MoveToRandomPointNear(const FVector& Center, float Radius, float Speed);
	void Attack(ATLSCharacter* Player, float DeltaSeconds);

	UPROPERTY()
	TObjectPtr<UAIPerceptionComponent> PerceptionComp;

	UPROPERTY()
	TObjectPtr<ATLSCreature> Creature;

	TWeakObjectPtr<ATLSCharacter> Target;

	ETLSAIState State = ETLSAIState::Dormant;
	bool bConfigured = false;
	bool bMoveIssued = false;
	bool bSightContact = false;

	float StateTimer = 0.f;
	float RepathTimer = 0.f;
	float AttackTimer = 0.f;
	float LoseSightTimer = 0.f;
	float LightExposure = 0.f;

	FVector Home = FVector::ZeroVector;
	FVector LastKnownLocation = FVector::ZeroVector;
	FVector NoiseLocation = FVector::ZeroVector;

	FAISenseID SightSenseID;
	FAISenseID HearingSenseID;
};
