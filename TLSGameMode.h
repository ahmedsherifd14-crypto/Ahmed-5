#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TLSGameMode.generated.h"

UCLASS()
class THELASTSIGNAL_API ATLSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATLSGameMode();

	virtual void StartPlay() override;
};
