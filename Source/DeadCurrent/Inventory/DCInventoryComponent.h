#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Save/DCPersistentTypes.h"
#include "DCInventoryComponent.generated.h"

class UDCItemDefinition;

/** A quantity of one item. Never exceeds the item's MaxStackSize. */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCItemStack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
	TObjectPtr<const UDCItemDefinition> Item;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory", meta=(ClampMin="1"))
	int32 Quantity = 1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDCInventoryChanged, UDCInventoryComponent*, Inventory);

/**
 *  Holds items as stacks. Used by the player, and by anything else that carries items (corpses, containers).
 */
UCLASS(ClassGroup=(DeadCurrent), meta=(BlueprintSpawnableComponent))
class DEADCURRENT_API UDCInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Adds up to Quantity of Item, topping up existing stacks first. Returns how many were added. */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	int32 AddItem(const UDCItemDefinition* Item, int32 Quantity = 1);

	/** Removes up to Quantity of Item. Returns how many were removed. */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	int32 RemoveItem(const UDCItemDefinition* Item, int32 Quantity = 1);

	/** Total quantity of Item across all stacks */
	UFUNCTION(BlueprintPure, Category="Inventory")
	int32 GetQuantity(const UDCItemDefinition* Item) const;

	UFUNCTION(BlueprintPure, Category="Inventory")
	int32 GetQuantityByItemId(FName ItemId) const;

	UFUNCTION(BlueprintPure, Category="Inventory")
	float GetTotalWeight() const;

	UFUNCTION(BlueprintPure, Category="Inventory")
	bool IsEmpty() const { return Stacks.IsEmpty(); }

	/**
	 *  Moves every stack into Destination. Returns how many items were transferred.
	 *  Used by corpses and containers.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	int32 TransferAllTo(UDCInventoryComponent* Destination);

	void CaptureStacks(TArray<FDCSavedItemStack>& OutStacks) const;

	void ReplaceFromSaved(const TArray<FDCSavedItemStack>& SavedStacks);

	const TArray<FDCItemStack>& GetStacks() const { return Stacks; }

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FDCInventoryChanged OnInventoryChanged;

protected:

	/** Current contents. Set on placed actors to give them starting items. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
	TArray<FDCItemStack> Stacks;
};
