#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DCSaveSubsystem.generated.h"

class ADCPlayerCharacter;
class UDCQuestComponent;
class UDCSaveGame;
class UDCWorldStateSubsystem;

/**
 *  Writes and reads the DeadCurrent save slot. F5 / F9 on the player controller call this.
 *  Loading reopens the saved map first, then applies the save once the fresh map has started
 *  (ADCGameMode::StartPlay), so a load in a running session matches a load after relaunching.
 */
UCLASS()
class DEADCURRENT_API UDCSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	static const FString DefaultSlotName;

	/** Slot F5/F9 use. Tests point this at a scratch slot so they never touch the player's save. */
	void SetSlotName(const FString& InSlotName) { SlotName = InSlotName; }

	const FString& GetSlotName() const { return SlotName; }

	UFUNCTION(BlueprintCallable, Category="Save")
	bool SaveCurrentGame();

	/** Reads the slot and opens its map. The save is applied by ApplyPendingLoad when the map starts. */
	UFUNCTION(BlueprintCallable, Category="Save")
	bool LoadCurrentGame();

	/** Applies a load requested by LoadCurrentGame. Called by the game mode once the map has begun play. */
	void ApplyPendingLoad(UWorld* World);

	bool HasPendingLoad() const { return PendingLoad != nullptr; }

	/** Quest log, world flags and discovered locations. Split out from the player so tests can round-trip them. */
	static void CaptureProgress(UDCSaveGame* Save, const UDCQuestComponent* Quests, const UDCWorldStateSubsystem* WorldState);

	/** World state first (ApplyWorldState), then the quest log. Neither re-runs stage consequences. */
	static void ApplyProgress(const UDCSaveGame* Save, UDCQuestComponent* Quests, UDCWorldStateSubsystem* WorldState);

	/** World flags and discovered locations, without broadcasting. */
	static void ApplyWorldState(const UDCSaveGame* Save, UDCWorldStateSubsystem* WorldState);

	/** Every registered persistent actor (existence, alive, doors, inventories). */
	static void CaptureWorld(UDCSaveGame* Save, UWorld* World);

	static void ApplyWorld(const UDCSaveGame* Save, UWorld* World);

private:

	void CapturePlayer(UDCSaveGame* Save, const ADCPlayerCharacter* Player) const;

	void ApplyPlayer(const UDCSaveGame* Save, ADCPlayerCharacter* Player) const;

	UPROPERTY()
	TObjectPtr<UDCSaveGame> PendingLoad;

	FString SlotName = DefaultSlotName;
};
