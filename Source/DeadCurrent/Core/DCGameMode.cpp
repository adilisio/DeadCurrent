#include "Core/DCGameMode.h"
#include "Engine/GameInstance.h"
#include "Save/DCSaveSubsystem.h"
#include "UI/DCHUD.h"

ADCGameMode::ADCGameMode()
{
	HUDClass = ADCHUD::StaticClass();
}

void ADCGameMode::StartPlay()
{
	Super::StartPlay();

	// Every actor has begun play (persistent ids registered), so a load requested before the map opened can apply.
	if (UDCSaveSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDCSaveSubsystem>() : nullptr)
	{
		Save->ApplyPendingLoad(GetWorld());
	}
}
