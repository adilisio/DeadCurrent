#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Save/DCPersistentTypes.h"

int32 UDCInventoryComponent::AddItem(const UDCItemDefinition* Item, int32 Quantity)
{
	if (!Item || Quantity <= 0)
	{
		return 0;
	}

	const int32 MaxStack = FMath::Max(1, Item->MaxStackSize);
	int32 Remaining = Quantity;

	for (FDCItemStack& Stack : Stacks)
	{
		if (Stack.Item == Item && Stack.Quantity < MaxStack)
		{
			const int32 Added = FMath::Min(Remaining, MaxStack - Stack.Quantity);
			Stack.Quantity += Added;
			Remaining -= Added;
			if (Remaining == 0)
			{
				break;
			}
		}
	}

	while (Remaining > 0)
	{
		const int32 Added = FMath::Min(Remaining, MaxStack);
		Stacks.Add({ Item, Added });
		Remaining -= Added;
	}

	OnInventoryChanged.Broadcast(this);
	return Quantity;
}

int32 UDCInventoryComponent::RemoveItem(const UDCItemDefinition* Item, int32 Quantity)
{
	if (!Item || Quantity <= 0)
	{
		return 0;
	}

	int32 Remaining = Quantity;

	// Take from the newest stacks first so older, full stacks stay intact.
	for (int32 Index = Stacks.Num() - 1; Index >= 0 && Remaining > 0; --Index)
	{
		FDCItemStack& Stack = Stacks[Index];
		if (Stack.Item != Item)
		{
			continue;
		}

		const int32 Removed = FMath::Min(Remaining, Stack.Quantity);
		Stack.Quantity -= Removed;
		Remaining -= Removed;
		if (Stack.Quantity == 0)
		{
			Stacks.RemoveAt(Index);
		}
	}

	const int32 Removed = Quantity - Remaining;
	if (Removed > 0)
	{
		OnInventoryChanged.Broadcast(this);
	}
	return Removed;
}

int32 UDCInventoryComponent::GetQuantity(const UDCItemDefinition* Item) const
{
	int32 Total = 0;
	for (const FDCItemStack& Stack : Stacks)
	{
		if (Stack.Item == Item)
		{
			Total += Stack.Quantity;
		}
	}
	return Total;
}

int32 UDCInventoryComponent::TransferAllTo(UDCInventoryComponent* Destination)
{
	if (!Destination || Destination == this || Stacks.IsEmpty())
	{
		return 0;
	}

	int32 Moved = 0;
	const TArray<FDCItemStack> Remaining = Stacks;
	for (const FDCItemStack& Stack : Remaining)
	{
		if (!Stack.Item)
		{
			continue;
		}

		const int32 Added = Destination->AddItem(Stack.Item, Stack.Quantity);
		if (Added > 0)
		{
			RemoveItem(Stack.Item, Added);
			Moved += Added;
		}
	}
	return Moved;
}

float UDCInventoryComponent::GetTotalWeight() const
{
	float Total = 0.0f;
	for (const FDCItemStack& Stack : Stacks)
	{
		if (Stack.Item)
		{
			Total += Stack.Item->Weight * Stack.Quantity;
		}
	}
	return Total;
}

void UDCInventoryComponent::CaptureStacks(TArray<FDCSavedItemStack>& OutStacks) const
{
	OutStacks.Reset();
	for (const FDCItemStack& Stack : Stacks)
	{
		if (!Stack.Item)
		{
			continue;
		}

		FDCSavedItemStack Saved;
		Saved.ItemId = Stack.Item->ItemId;
		Saved.Item = const_cast<UDCItemDefinition*>(Stack.Item.Get());
		Saved.Quantity = Stack.Quantity;
		OutStacks.Add(Saved);
	}
}

void UDCInventoryComponent::ReplaceFromSaved(const TArray<FDCSavedItemStack>& SavedStacks)
{
	Stacks.Empty();
	for (const FDCSavedItemStack& Saved : SavedStacks)
	{
		const UDCItemDefinition* Item = Saved.Item.LoadSynchronous();
		if (!Item)
		{
			Item = UDCItemDefinition::FindByItemId(Saved.ItemId);
		}
		if (Item && Saved.Quantity > 0)
		{
			AddItem(Item, Saved.Quantity);
		}
	}

	if (SavedStacks.IsEmpty())
	{
		OnInventoryChanged.Broadcast(this);
	}
}
