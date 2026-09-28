#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
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

	return true;
}

#endif
