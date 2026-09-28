#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Kismet/GameplayStatics.h"
#include "Save/DCPersistentTypes.h"
#include "Save/DCSaveGame.h"
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

	Saved[0].ItemPath.Reset();

	UDCInventoryComponent* Dest = NewObject<UDCInventoryComponent>();
	Dest->AddItem(Ammo, 3);
	Dest->ReplaceFromSaved(Saved);
	TestEqual(TEXT("FindByItemId restores quantity"), Dest->GetQuantity(Ammo), 7);

	Dest->ReplaceFromSaved(TArray<FDCSavedItemStack>());
	TestTrue(TEXT("Empty snapshot clears dest"), Dest->IsEmpty());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCSaveWorldInventorySlotTest, "DeadCurrent.Save.WorldInventorySlot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCSaveWorldInventorySlotTest::RunTest(const FString& Parameters)
{
	const FString Slot = TEXT("DeadCurrent_TestWorldInv");
	UGameplayStatics::DeleteGameInSlot(Slot, 0);

	UDCItemDefinition* Ammo = NewObject<UDCItemDefinition>();
	Ammo->ItemId = TEXT("test.ammo");
	Ammo->MaxStackSize = 30;
	UDCItemDefinition* DressingDef = NewObject<UDCItemDefinition>();
	DressingDef->ItemId = TEXT("test.dressing");
	UDCItemDefinition* WiringDef = NewObject<UDCItemDefinition>();
	WiringDef->ItemId = TEXT("test.wiring");
	WiringDef->MaxStackSize = 10;
	Ammo->AddToRoot();
	DressingDef->AddToRoot();
	WiringDef->AddToRoot();

	auto UnrootItems = [Ammo, DressingDef, WiringDef]()
	{
		Ammo->RemoveFromRoot();
		DressingDef->RemoveFromRoot();
		WiringDef->RemoveFromRoot();
	};

	UDCInventoryComponent* Corpse = NewObject<UDCInventoryComponent>();
	Corpse->AddItem(Ammo, 12);
	Corpse->AddItem(DressingDef, 1);
	Corpse->AddItem(WiringDef, 2);

	UDCInventoryComponent* Player = NewObject<UDCInventoryComponent>();
	TestEqual(TEXT("Took first corpse stack"), Corpse->TransferFirstStackTo(Player), 12);
	TestEqual(TEXT("Two stacks remain on corpse"), Corpse->GetStacks().Num(), 2);

	TArray<FDCSavedItemStack> Remaining;
	Corpse->CaptureStacks(Remaining);
	TestEqual(TEXT("Captured remaining stacks"), Remaining.Num(), 2);

	UDCSaveGame* Save = Cast<UDCSaveGame>(UGameplayStatics::CreateSaveGameObject(UDCSaveGame::StaticClass()));
	FDCPersistentActorState Actor;
	Actor.PersistentId = TEXT("boat.scavenger");
	Actor.bExists = true;
	Actor.bAlive = false;
	Actor.Inventory = Remaining;
	Save->WorldActors.Add(Actor);

	FDCSavedActorInventory ActorInventory;
	ActorInventory.PersistentId = Actor.PersistentId;
	ActorInventory.Stacks = Remaining;
	Save->ActorInventories.Add(ActorInventory);

	Save->WorldInvActorIds.Add(Actor.PersistentId);
	for (const FDCSavedItemStack& Stack : Remaining)
	{
		Save->WorldInvStackActorIds.Add(Actor.PersistentId);
		Save->WorldInvItemIds.Add(Stack.ItemId);
		Save->WorldInvQuantities.Add(Stack.Quantity);
		Save->WorldInvItemPaths.Add(Stack.ItemPath);
	}

	TestTrue(TEXT("Wrote test slot"), UGameplayStatics::SaveGameToSlot(Save, Slot, 0));

	UDCSaveGame* Loaded = Cast<UDCSaveGame>(UGameplayStatics::LoadGameFromSlot(Slot, 0));
	TestTrue(TEXT("Loaded test slot"), Loaded != nullptr);
	if (!Loaded)
	{
		UnrootItems();
		return false;
	}

	TestEqual(TEXT("Flat captured actors round-trip"), Loaded->WorldInvActorIds.Num(), 1);
	TestEqual(TEXT("Flat remaining stacks round-trip"), Loaded->WorldInvItemIds.Num(), 2);
	if (Loaded->WorldInvItemIds.Num() >= 2)
	{
		TestEqual(TEXT("Dressing id"), Loaded->WorldInvItemIds[0], FName(TEXT("test.dressing")));
		TestEqual(TEXT("Wiring id"), Loaded->WorldInvItemIds[1], FName(TEXT("test.wiring")));
		TestEqual(TEXT("Wiring qty"), Loaded->WorldInvQuantities[1], 2);
	}

	TArray<FDCSavedItemStack> RestoredStacks;
	const int32 Num = FMath::Min(Loaded->WorldInvItemIds.Num(), Loaded->WorldInvQuantities.Num());
	for (int32 Index = 0; Index < Num; ++Index)
	{
		FDCSavedItemStack Stack;
		Stack.ItemId = Loaded->WorldInvItemIds[Index];
		Stack.Quantity = Loaded->WorldInvQuantities[Index];
		if (Loaded->WorldInvItemPaths.IsValidIndex(Index))
		{
			Stack.ItemPath = Loaded->WorldInvItemPaths[Index];
		}
		RestoredStacks.Add(Stack);
	}

	UDCInventoryComponent* AfterLoad = NewObject<UDCInventoryComponent>();
	AfterLoad->AddItem(Ammo, 12);
	AfterLoad->AddItem(DressingDef, 1);
	AfterLoad->AddItem(WiringDef, 2);
	AfterLoad->ReplaceFromSaved(RestoredStacks);
	TestEqual(TEXT("Ammo stayed looted"), AfterLoad->GetQuantityByItemId(TEXT("test.ammo")), 0);
	TestEqual(TEXT("Dressing remains"), AfterLoad->GetQuantityByItemId(TEXT("test.dressing")), 1);
	TestEqual(TEXT("Wiring remains"), AfterLoad->GetQuantityByItemId(TEXT("test.wiring")), 2);
	TestEqual(TEXT("Two stacks still lootable"), AfterLoad->GetStacks().Num(), 2);

	UGameplayStatics::DeleteGameInSlot(Slot, 0);
	UnrootItems();
	return true;
}

#endif
