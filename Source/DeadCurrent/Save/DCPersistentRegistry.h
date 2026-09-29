#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DCPersistentRegistry.generated.h"

/**
 *  Looks up placed world actors by persistent ID for the current map.
 */
UCLASS()
class DEADCURRENT_API UDCPersistentRegistry : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	/** Returns false if Id is none or already registered to a different live actor. */
	bool RegisterActor(AActor* Actor, FName Id);

	void UnregisterActor(AActor* Actor);

	UFUNCTION(BlueprintPure, Category="Save")
	AActor* FindActor(FName Id) const;

	UFUNCTION(BlueprintPure, Category="Save")
	TArray<FName> GetRegisteredIds() const;

	/**
	 *  The actor with this id was removed during play (a taken pickup). Recorded on
	 *  EndPlay(Destroyed), not on map travel, so a later save can name it explicitly.
	 */
	void NoteRemoved(FName Id);

	/** Replaces the removed set. Used when a save is applied, before actors are destroyed again. */
	void RestoreRemovedIds(const TArray<FName>& Ids);

	TArray<FName> GetRemovedIds() const;

private:

	TMap<FName, TWeakObjectPtr<AActor>> ById;

	TSet<FName> RemovedIds;
};
