#pragma once

#include "CoreMinimal.h"
#include "DCPersistentTypes.generated.h"

/** One inventory stack in a save. Item is resolved by ItemId, then ItemPath. */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCSavedItemStack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	FName ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	int32 Quantity = 1;

	/** Package path, e.g. /Game/Items/DA_Item_Ammo9mm.DA_Item_Ammo9mm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	FString ItemPath;
};

/** World-actor inventory stored at save-game root so it survives USaveGame serialization. */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCSavedActorInventory
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	FName PersistentId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	TArray<FDCSavedItemStack> Stacks;
};

/** One quest's current stage in a save. */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCSavedQuestState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	FName QuestId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	FName StageId;
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
