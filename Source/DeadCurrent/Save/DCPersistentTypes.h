#pragma once

#include "CoreMinimal.h"
#include "DCPersistentTypes.generated.h"

/**
 *  Snapshot of one persistent world actor. FP-13 will store this in the save game.
 *  Add fields as systems need them; lookup is always by PersistentId.
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
};
