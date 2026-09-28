#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "DCPlayerCameraManager.generated.h"

/**
 *  First person camera manager.
 *  Limits min/max look pitch.
 */
UCLASS()
class DEADCURRENT_API ADCPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ADCPlayerCameraManager();
};
