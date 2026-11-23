#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TLSLevelDirector.generated.h"

class USoundBase;

/**
 * Place one of these in each level. It holds the level's scare sounds and tuning, and the
 * horror director reads it on level start. Lights that should flicker are tagged "TLS_Flicker".
 */
UCLASS()
class THELASTSIGNAL_API ATLSLevelDirector : public AActor
{
	GENERATED_BODY()

public:
	ATLSLevelDirector();

	/** Unexpected sounds played at a distance behind or beside the player while tension is moderate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scares")
	TArray<TObjectPtr<USoundBase>> AmbientScares;

	/** Named scares triggered from Blueprint volumes via the director's RequestScare. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scares")
	TMap<FName, TObjectPtr<USoundBase>> NamedScares;

	/** Seconds between ambient scares at moderate tension. Randomised between the two values. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scares", meta = (ClampMin = "5"))
	FVector2D AmbientScareInterval = FVector2D(25.f, 60.f);

	/** Multiplier on how readily tension rises in this level. 1 = default. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tension", meta = (ClampMin = "0"))
	float TensionScale = 1.f;

	/** Lights flicker only above this tension. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tension", meta = (ClampMin = "0", ClampMax = "1"))
	float FlickerThreshold = 0.3f;
};
