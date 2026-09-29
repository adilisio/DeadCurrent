#include "Core/DCTestHelpers.h"
#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Items/DCItemPickup.h"
#include "Kismet/GameplayStatics.h"
#include "Save/DCPersistentIdComponent.h"
#include "Save/DCPersistentRegistry.h"
#include "Save/DCPersistentTypes.h"
#include "Save/DCSaveGame.h"
#include "Save/DCSaveSubsystem.h"
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

namespace DCPickupSaveTest
{
	ADCItemPickup* SpawnPickup(UWorld* World, FName Id)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ADCItemPickup* Pickup = World->SpawnActorDeferred<ADCItemPickup>(
			ADCItemPickup::StaticClass(), FTransform::Identity, nullptr, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (UDCPersistentIdComponent* Comp = Pickup->FindComponentByClass<UDCPersistentIdComponent>())
		{
			Comp->SetPersistentId(Id);
		}
		return Cast<ADCItemPickup>(UGameplayStatics::FinishSpawningActor(Pickup, FTransform::Identity));
	}
}

/**
 *  Taking a pickup records its id. A pickup added to the map after the save, and therefore
 *  absent from it, is left in place. Older saves still remove only the pickups that existed then.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCSaveRemovedPickupTest, "DeadCurrent.Save.RemovedPickup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCSaveRemovedPickupTest::RunTest(const FString& Parameters)
{
	using namespace DCPickupSaveTest;

	UDCSaveGame* Save = nullptr;
	{
		FDCTestWorld Source;
		ADCItemPickup* Taken = SpawnPickup(Source.Get(), TEXT("test.taken"));
		ADCItemPickup* Kept = SpawnPickup(Source.Get(), TEXT("test.kept"));
		TestNotNull(TEXT("Taken pickup registered"), Source.Get()->GetSubsystem<UDCPersistentRegistry>()->FindActor(TEXT("test.taken")));
		TestNotNull(TEXT("Kept pickup registered"), Source.Get()->GetSubsystem<UDCPersistentRegistry>()->FindActor(TEXT("test.kept")));
		Taken->Destroy();
		TestNull(TEXT("Taken pickup left the registry"), Source.Get()->GetSubsystem<UDCPersistentRegistry>()->FindActor(TEXT("test.taken")));
		TestTrue(TEXT("Kept pickup still registered"), Source.Get()->GetSubsystem<UDCPersistentRegistry>()->FindActor(TEXT("test.kept")) == Kept);

		Save = NewObject<UDCSaveGame>();
		Save->SaveVersion = UDCSaveGame::CurrentVersion;
		Save->AddToRoot();
		UDCSaveSubsystem::CaptureWorld(Save, Source.Get());
	}

	TestTrue(TEXT("Save names the taken pickup"), Save->RemovedPersistentIds.Contains(TEXT("test.taken")));
	TestFalse(TEXT("Save does not name the kept pickup"), Save->RemovedPersistentIds.Contains(TEXT("test.kept")));

	{
		FDCTestWorld Dest;
		ADCItemPickup* TakenAgain = SpawnPickup(Dest.Get(), TEXT("test.taken"));
		ADCItemPickup* KeptAgain = SpawnPickup(Dest.Get(), TEXT("test.kept"));
		ADCItemPickup* AddedLater = SpawnPickup(Dest.Get(), TEXT("test.added_later"));
		UDCSaveSubsystem::ApplyWorld(Save, Dest.Get());

		TestFalse(TEXT("Taken pickup stays gone"), IsValid(TakenAgain));
		TestTrue(TEXT("Pickup present at save stays"), IsValid(KeptAgain));
		TestTrue(TEXT("Pickup added after the save stays"), IsValid(AddedLater));
		TestNull(TEXT("Taken id is not registered"), Dest.Get()->GetSubsystem<UDCPersistentRegistry>()->FindActor(TEXT("test.taken")));
		TestNotNull(TEXT("New id is registered"), Dest.Get()->GetSubsystem<UDCPersistentRegistry>()->FindActor(TEXT("test.added_later")));
	}

	// Version 0 predates the coil. An absent coil was not taken; it did not exist yet.
	// An absent bench pistol did exist, and was taken.
	{
		UDCSaveGame* Legacy = NewObject<UDCSaveGame>();
		Legacy->SaveVersion = 0;
		FDCTestWorld World;
		ADCItemPickup* Pistol = SpawnPickup(World.Get(), TEXT("boat.pickup_pistol"));
		ADCItemPickup* Coil = SpawnPickup(World.Get(), TEXT("boat.pickup_coil"));
		ADCItemPickup* Future = SpawnPickup(World.Get(), TEXT("boat.pickup_future"));
		UDCSaveSubsystem::ApplyWorld(Legacy, World.Get());
		TestFalse(TEXT("v0: bench pistol absent from the save was taken"), IsValid(Pistol));
		TestTrue(TEXT("v0: coil postdates the save and stays"), IsValid(Coil));
		TestTrue(TEXT("v0: a pickup added even later stays"), IsValid(Future));
	}

	// Version 3 includes the coil. Absence means it was taken. A newer id still stays.
	{
		UDCSaveGame* Micro = NewObject<UDCSaveGame>();
		Micro->SaveVersion = 3;
		FDCPersistentActorState PistolState;
		PistolState.PersistentId = TEXT("boat.pickup_pistol");
		PistolState.bExists = true;
		Micro->WorldActors.Add(PistolState);

		FDCTestWorld World;
		ADCItemPickup* Pistol = SpawnPickup(World.Get(), TEXT("boat.pickup_pistol"));
		ADCItemPickup* Coil = SpawnPickup(World.Get(), TEXT("boat.pickup_coil"));
		ADCItemPickup* Future = SpawnPickup(World.Get(), TEXT("boat.pickup_future"));
		UDCSaveSubsystem::ApplyWorld(Micro, World.Get());
		TestTrue(TEXT("v3: pistol listed in the save stays"), IsValid(Pistol));
		TestFalse(TEXT("v3: coil absent from the save was taken"), IsValid(Coil));
		TestTrue(TEXT("v3: a pickup the save could not have known stays"), IsValid(Future));
	}

	Save->RemoveFromRoot();
	return true;
}

#endif
