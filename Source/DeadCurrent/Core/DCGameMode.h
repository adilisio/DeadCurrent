#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DCGameMode.generated.h"

/**
 *  Base game mode for DEAD CURRENT.
 */
UCLASS(abstract)
class DEADCURRENT_API ADCGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADCGameMode();

	virtual void StartPlay() override;
};
