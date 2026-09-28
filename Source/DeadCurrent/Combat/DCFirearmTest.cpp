#include "Combat/DCFirearm.h"
#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCFirearmAmmoTest, "DeadCurrent.Combat.FirearmAmmo",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCFirearmAmmoTest::RunTest(const FString& Parameters)
{
	UDCItemDefinition* Ammo = NewObject<UDCItemDefinition>();
	Ammo->MaxStackSize = 99;

	UDCItemDefinition* Def = NewObject<UDCItemDefinition>();
	Def->MagazineSize = 5;
	Def->AmmoItem = Ammo;

	UDCInventoryComponent* Inventory = NewObject<UDCInventoryComponent>();
	ADCFirearm* Firearm = NewObject<ADCFirearm>();
	Firearm->SetDefinition(Def);

	TestEqual(TEXT("Empty mag on equip"), Firearm->GetRoundsInMagazine(), 0);
	TestEqual(TEXT("Reload with no ammo does nothing"), Firearm->LoadRoundsFromInventory(Inventory), 0);

	Inventory->AddItem(Ammo, 8);
	TestEqual(TEXT("Reload fills the mag from inventory"), Firearm->LoadRoundsFromInventory(Inventory), 5);
	TestEqual(TEXT("Mag is full"), Firearm->GetRoundsInMagazine(), 5);
	TestEqual(TEXT("Reserve is leftover"), Inventory->GetQuantity(Ammo), 3);
	TestEqual(TEXT("Topping up a full mag takes nothing"), Firearm->LoadRoundsFromInventory(Inventory), 0);

	TestEqual(TEXT("Consume one"), Firearm->ConsumeRound(), 1);
	TestEqual(TEXT("Mag after one shot"), Firearm->GetRoundsInMagazine(), 4);

	for (int32 i = 0; i < 4; ++i)
	{
		Firearm->ConsumeRound();
	}
	TestEqual(TEXT("Empty mag"), Firearm->GetRoundsInMagazine(), 0);
	TestEqual(TEXT("Cannot consume from empty"), Firearm->ConsumeRound(), 0);

	TestEqual(TEXT("Partial reload takes remaining reserve"), Firearm->LoadRoundsFromInventory(Inventory), 3);
	TestEqual(TEXT("Mag after partial reload"), Firearm->GetRoundsInMagazine(), 3);
	TestEqual(TEXT("Reserve empty"), Inventory->GetQuantity(Ammo), 0);
	TestEqual(TEXT("Cannot fire interval without a world"), Firearm->CanFire(), false);

	return true;
}

#endif
