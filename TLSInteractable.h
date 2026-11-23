#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/Interface.h"
#include "TLSInteractable.generated.h"

class ATLSCharacter;
class UStaticMeshComponent;

UINTERFACE(MinimalAPI, Blueprintable)
class UTLSInteractable : public UInterface
{
	GENERATED_BODY()
};

/** Anything the player can press E on. */
class THELASTSIGNAL_API ITLSInteractable
{
	GENERATED_BODY()

public:
	virtual FText GetInteractPrompt(const ATLSCharacter* Interactor) const = 0;
	virtual void OnInteract(ATLSCharacter* Interactor) = 0;
};

/** Base actor with a mesh and a noise hook. Subclasses override the prompt and interaction. */
UCLASS(Abstract)
class THELASTSIGNAL_API ATLSInteractableActor : public AActor, public ITLSInteractable
{
	GENERATED_BODY()

public:
	ATLSInteractableActor();

	virtual FText GetInteractPrompt(const ATLSCharacter* Interactor) const override;

	/** Loudness reported to AI hearing when this object is used. 0 = silent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise", meta = (ClampMin = "0"))
	float InteractNoiseLoudness = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText PromptText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** Unique ID for things that must stay gone after a checkpoint reload (pickups, documents). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
	FName PersistentId;

	/** Consumed objects remove themselves on BeginPlay. */
	UFUNCTION(BlueprintPure, Category = "Persistence")
	bool IsConsumed() const;

	UFUNCTION(BlueprintCallable, Category = "Persistence")
	void MarkConsumed();

	virtual void BeginPlay() override;

protected:
	void ReportNoise(const ATLSCharacter* Source) const;
};
