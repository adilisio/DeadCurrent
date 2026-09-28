#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Save/DCPersistentTypes.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCInventoryStackingTest, "DeadCurrent.Inventory.Stacking",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCInventoryStackingTest::RunTest(const FString& Parameters)
{
	UDCItemDefinition* Ammo = NewObject<UDCItemDefinition>();
	Ammo->MaxStackSize = 10;
	Ammo->Weight = 0.5f;

	UDCItemDefinition* Pistol = NewObject<UDCItemDefinition>();
	Pistol->MaxStackSize = 1;
	Pistol->Weight = 1.0f;

	UDCInventoryComponent* Inventory = NewObject<UDCInventoryComponent>();

	TestEqual(TEXT("Add returns amount added"), Inventory->AddItem(Ammo, 7), 7);
	TestEqual(TEXT("Topping up spills into a new stack"), Inventory->AddItem(Ammo, 8), 8);
	TestEqual(TEXT("Two ammo stacks"), Inventory->GetStacks().Num(), 2);
	TestEqual(TEXT("First stack is full"), Inventory->GetStacks()[0].Quantity, 10);
	TestEqual(TEXT("Ammo quantity"), Inventory->GetQuantity(Ammo), 15);

	Inventory->AddItem(Pistol, 2);
	TestEqual(TEXT("Unstackable items take a stack each"), Inventory->GetStacks().Num(), 4);
	TestEqual(TEXT("Total weight"), Inventory->GetTotalWeight(), 15 * 0.5f + 2 * 1.0f);

	TestEqual(TEXT("Remove returns amount removed"), Inventory->RemoveItem(Ammo, 6), 6);
	TestEqual(TEXT("Emptied stack is dropped"), Inventory->GetStacks().Num(), 3);
	TestEqual(TEXT("Ammo quantity after remove"), Inventory->GetQuantity(Ammo), 9);
	TestEqual(TEXT("Cannot remove more than held"), Inventory->RemoveItem(Pistol, 5), 2);
	TestEqual(TEXT("Pistols gone"), Inventory->GetQuantity(Pistol), 0);
	TestEqual(TEXT("Invalid add is rejected"), Inventory->AddItem(nullptr, 3), 0);
	TestEqual(TEXT("Zero add is rejected"), Inventory->AddItem(Ammo, 0), 0);

	UDCInventoryComponent* Source = NewObject<UDCInventoryComponent>();
	UDCInventoryComponent* Other = NewObject<UDCInventoryComponent>();
	Source->AddItem(Ammo, 4);
	Source->AddItem(Pistol, 1);
	TestEqual(TEXT("Transfer moves every item"), Source->TransferAllTo(Other), 5);
	TestTrue(TEXT("Source is empty after transfer"), Source->IsEmpty());
	TestEqual(TEXT("Destination received ammo"), Other->GetQuantity(Ammo), 4);
	TestEqual(TEXT("Destination received pistol"), Other->GetQuantity(Pistol), 1);
	TestEqual(TEXT("Transfer to self is rejected"), Other->TransferAllTo(Other), 0);
	TestEqual(TEXT("Transfer to null is rejected"), Other->TransferAllTo(nullptr), 0);

	Ammo->ItemId = TEXT("ammo.9mm");
	Pistol->ItemId = TEXT("weapon.pistol");
	UDCInventoryComponent* Snapshot = NewObject<UDCInventoryComponent>();
	Snapshot->AddItem(Ammo, 9);
	Snapshot->AddItem(Pistol, 1);
	TArray<FDCSavedItemStack> Saved;
	Snapshot->CaptureStacks(Saved);
	TestEqual(TEXT("Capture writes two stacks"), Saved.Num(), 2);

	UDCInventoryComponent* Restored = NewObject<UDCInventoryComponent>();
	Restored->AddItem(Ammo, 99);
	Restored->ReplaceFromSaved(Saved);
	TestEqual(TEXT("Restore replaces rather than adding"), Restored->GetQuantity(Ammo), 9);
	TestEqual(TEXT("Restore keeps the pistol"), Restored->GetQuantity(Pistol), 1);

	Restored->ReplaceFromSaved(TArray<FDCSavedItemStack>());
	TestTrue(TEXT("Empty save clears inventory"), Restored->IsEmpty());

	return true;
}

#endif
