#pragma once

#include "CoreMinimal.h"
#include "DCPersistentTypes.generated.h"

class UDCItemDefinition;

/** One inventory stack in a save. Item is resolved by ItemId, with a soft path fallback. */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCSavedItemStack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	FName ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	TSoftObjectPtr<UDCItemDefinition> Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	int32 Quantity = 1;
};

/**
 *  Snapshot of one persistent world actor. Lookup is always by PersistentId.
 */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCPersistentActorState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	FName PersistentId;

	/** False when a pickup was taken or an actor was removed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	bool bExists = true;

	/** Characters: still alive. Ignored for props. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	bool bAlive = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	bool bIsOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	float DoorYaw = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	TArray<FDCSavedItemStack> Inventory;
};
