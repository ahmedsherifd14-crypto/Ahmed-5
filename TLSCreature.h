#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TLSCreature.generated.h"

class UCapsuleComponent;
class USoundBase;

/** Behaviour profile. Each type reacts to the player differently. */
UENUM(BlueprintType)
enum class ETLSCreatureType : uint8
{
	Stalker,   // Hunts by sight. Recoils from the flashlight. Patient, persistent.
	Hunter,    // Follows sound and gives chase quickly. Ignores light.
	Lurker     // Only attacks when the player is in the dark. Hides in rooms.
};

/**
 * The entity. Movement and attacks are driven by ATLSCreatureController; this class holds the
 * tuning and cosmetic hooks (vocals, footsteps) so the Blueprint can supply meshes and cues.
 */
UCLASS()
class THELASTSIGNAL_API ATLSCreature : public ACharacter
{
	GENERATED_BODY()

public:
	ATLSCreature();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	ETLSCreatureType CreatureType = ETLSCreatureType::Stalker;

	/** Start idle until something is sensed. Good for creatures sleeping in a room. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	bool bStartDormant = true;

	/** Radius around the spawn point the creature roams while patrolling. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature", meta = (ClampMin = "100"))
	float PatrolRadius = 1800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	float PatrolWalkSpeed = 140.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	float HuntSpeed = 520.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	float InvestigateSpeed = 260.f;

	/** Sight range for this type. BeginPlay overrides it from CreatureType; edit in Blueprint to fine-tune. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	float SightRange = 2600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	float HearingRange = 3200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	float AttackRange = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	float AttackDamage = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	float AttackInterval = 1.2f;

	/** Seconds of sustained flashlight exposure before the creature recoils. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	float LightRecoilThreshold = 1.2f;

	/** Seconds spent retreating after a recoil. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	float RetreatDuration = 6.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TArray<TObjectPtr<USoundBase>> Vocals;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> RecoilSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> AttackSound;

	/** Called by the controller when the creature changes behaviour. Blueprint can animate here. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Creature")
	void OnBehaviourChanged(const FString& NewBehaviour);

	/** Plays a random vocal from Vocals at the creature's location. */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayRandomVocal();

	void PlayRecoil();
	void PlayAttack();
};
