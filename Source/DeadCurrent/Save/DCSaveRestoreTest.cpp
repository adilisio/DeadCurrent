#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Save/DCPersistentTypes.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCSaveInventoryRestoreTest, "DeadCurrent.Save.InventoryRestore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCSaveInventoryRestoreTest::RunTest(const FString& Parameters)
{
	UDCItemDefinition* Ammo = NewObject<UDCItemDefinition>();
	Ammo->ItemId = TEXT("ammo.restore");
	Ammo->MaxStackSize = 20;

	UDCInventoryComponent* Source = NewObject<UDCInventoryComponent>();
	Source->AddItem(Ammo, 7);

	TArray<FDCSavedItemStack> Saved;
	Source->CaptureStacks(Saved);
	TestEqual(TEXT("Captured one stack"), Saved.Num(), 1);
	TestEqual(TEXT("Captured id"), Saved[0].ItemId, FName(TEXT("ammo.restore")));

	Saved[0].Item.Reset();

	UDCInventoryComponent* Dest = NewObject<UDCInventoryComponent>();
	Dest->AddItem(Ammo, 3);
	Dest->ReplaceFromSaved(Saved);
	TestEqual(TEXT("FindByItemId restores quantity"), Dest->GetQuantity(Ammo), 7);

	Dest->ReplaceFromSaved(TArray<FDCSavedItemStack>());
	TestTrue(TEXT("Empty snapshot clears dest"), Dest->IsEmpty());

	return true;
}

#endif
