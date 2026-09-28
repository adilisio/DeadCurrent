#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"

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
