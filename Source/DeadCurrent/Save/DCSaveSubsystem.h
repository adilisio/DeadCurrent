#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DCSaveSubsystem.generated.h"

class ADCPlayerCharacter;
class UDCSaveGame;

/**
 *  Writes and reads the DeadCurrent save slot. F5 / F9 on the player controller call this.
 */
UCLASS()
class DEADCURRENT_API UDCSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	static const FString SlotName;

	UFUNCTION(BlueprintCallable, Category="Save")
	bool SaveCurrentGame();

	UFUNCTION(BlueprintCallable, Category="Save")
	bool LoadCurrentGame();

	static FString MakeSlotName() { return SlotName; }

private:

	void CapturePlayer(UDCSaveGame* Save, const ADCPlayerCharacter* Player) const;

	void ApplyPlayer(UDCSaveGame* Save, ADCPlayerCharacter* Player) const;

	void CaptureWorld(UDCSaveGame* Save, UWorld* World) const;

	void ApplyWorld(UDCSaveGame* Save, UWorld* World) const;
};
