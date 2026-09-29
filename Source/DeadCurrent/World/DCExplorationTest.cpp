#include "Core/DCTestHelpers.h"
#include "Interaction/DCInteractable.h"
#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ScopeExit.h"
#include "Save/DCPersistentIdComponent.h"
#include "Save/DCSaveGame.h"
#include "Save/DCSaveSubsystem.h"
#include "World/DCLocationVolume.h"
#include "World/DCLootContainer.h"
#include "World/DCWorldStateSubsystem.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 *  Exploration loop systems, independent of any map: location discovery, loot containers,
 *  world-conditioned hazards and lights. DeadCurrent.Map.Boathouse.SurveyLaunch* covers the real POI.
 */
namespace DCExplorationTest
{
	ADCLocationVolume* SpawnVolume(const FDCTestWorld& World, FName Id, const TCHAR* Name, const FTransform& Transform = FTransform::Identity)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ADCLocationVolume* Volume = World.Get()->SpawnActor<ADCLocationVolume>(ADCLocationVolume::StaticClass(), Transform, Params);
		Volume->SetLocation(Id, FText::FromString(Name));
		return Volume;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCLocationDiscoveryTest, "DeadCurrent.Exploration.Discovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCLocationDiscoveryTest::RunTest(const FString& Parameters)
{
	using namespace DCExplorationTest;
	const FString Slot = TEXT("DeadCurrent_TestDiscovery");
	UGameplayStatics::DeleteGameInSlot(Slot, 0);
	const FName Place = TEXT("test.wreck");

	UDCSaveGame* Save = NewObject<UDCSaveGame>();
	{
		FDCTestWorld World;
		AActor* Player = World.SpawnActor();
		UDCWorldStateSubsystem* WorldState = World.Get()->GetSubsystem<UDCWorldStateSubsystem>();
		int32 Announced = 0;
		int32 Changed = 0;
		WorldState->OnLocationDiscovered.AddLambda([&Announced, Place](FName Id) { Announced += Id == Place ? 1 : 0; });
		WorldState->OnChanged.AddLambda([&Changed]() { ++Changed; });

		// The volume is a box (default 500 x 500 x 300 half extent); rotation is respected.
		const FTransform Turned(FRotator(0.0, 45.0, 0.0), FVector(1000.0, 0.0, 0.0));
		ADCLocationVolume* Volume = SpawnVolume(World, Place, TEXT("Test Wreck"), Turned);
		TestTrue(TEXT("Contains its center"), Volume->ContainsPoint(FVector(1000.0, 0.0, 0.0)));
		TestTrue(TEXT("Contains a point only the turned box covers"), Volume->ContainsPoint(FVector(1000.0, 600.0, 0.0)));
		TestFalse(TEXT("Excludes the unturned corner"), Volume->ContainsPoint(FVector(1000.0 + 480.0, -480.0, 0.0)));
		TestFalse(TEXT("Excludes points above"), Volume->ContainsPoint(FVector(1000.0, 0.0, 400.0)));

		// Discovery happens once, however many times or volumes report it.
		TestFalse(TEXT("Not discovered yet"), Volume->IsDiscovered());
		TestTrue(TEXT("First discovery"), Volume->TryDiscover(Player));
		TestFalse(TEXT("Second discovery ignored"), Volume->TryDiscover(Player));
		ADCLocationVolume* SecondEntrance = SpawnVolume(World, Place, TEXT("Test Wreck"));
		TestFalse(TEXT("Another volume for the same place does not rediscover it"), SecondEntrance->TryDiscover(Player));
		TestTrue(TEXT("Discovered"), Volume->IsDiscovered() && WorldState->IsLocationDiscovered(Place));
		TestEqual(TEXT("Announced once"), Announced, 1);
		TestEqual(TEXT("Quests are told once"), Changed, 1);
		TestFalse(TEXT("A volume with no id is never discovered"), SpawnVolume(World, NAME_None, TEXT("Nowhere"))->TryDiscover(Player));
		TestEqual(TEXT("Display name lookup"), ADCLocationVolume::FindDisplayName(World.Get(), Place).ToString(), FString(TEXT("Test Wreck")));
		TestEqual(TEXT("Unknown id falls back to the id"), ADCLocationVolume::FindDisplayName(World.Get(), TEXT("test.none")).ToString(), FString(TEXT("test.none")));

		WorldState->SetFlag(TEXT("test.flag"));
		UDCSaveSubsystem::CaptureProgress(Save, nullptr, WorldState);
	}

	TestTrue(TEXT("Wrote slot"), UGameplayStatics::SaveGameToSlot(Save, Slot, 0));
	const UDCSaveGame* Loaded = Cast<UDCSaveGame>(UGameplayStatics::LoadGameFromSlot(Slot, 0));
	UGameplayStatics::DeleteGameInSlot(Slot, 0);
	if (!TestNotNull(TEXT("Read slot"), Loaded))
	{
		return false;
	}
	TestEqual(TEXT("One location saved"), Loaded->DiscoveredLocations.Num(), 1);

	// A fresh world (as after F9 reopening the map) restores the discovery silently.
	{
		FDCTestWorld World;
		AActor* Player = World.SpawnActor();
		UDCWorldStateSubsystem* WorldState = World.Get()->GetSubsystem<UDCWorldStateSubsystem>();
		int32 Announced = 0;
		WorldState->OnLocationDiscovered.AddLambda([&Announced](FName) { ++Announced; });
		ADCLocationVolume* Volume = SpawnVolume(World, Place, TEXT("Test Wreck"));
		TestFalse(TEXT("Fresh world: not discovered"), Volume->IsDiscovered());

		UDCSaveSubsystem::ApplyProgress(Loaded, nullptr, WorldState);
		TestTrue(TEXT("Restored: discovered"), Volume->IsDiscovered());
		TestTrue(TEXT("Restored: flags too"), WorldState->HasFlag(TEXT("test.flag")));
		TestFalse(TEXT("Restored: not rediscovered"), Volume->TryDiscover(Player));
		TestEqual(TEXT("Restore does not announce"), Announced, 0);

		// A save without locations (older format) clears them.
		UDCSaveSubsystem::ApplyWorldState(NewObject<UDCSaveGame>(), WorldState);
		TestFalse(TEXT("Older save: nothing discovered"), Volume->IsDiscovered());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCLootContainerTest, "DeadCurrent.Exploration.Container",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCLootContainerTest::RunTest(const FString& Parameters)
{
	const FString Slot = TEXT("DeadCurrent_TestContainer");
	UGameplayStatics::DeleteGameInSlot(Slot, 0);

	UDCItemDefinition* Ammo = NewObject<UDCItemDefinition>();
	Ammo->ItemId = TEXT("test.crate_ammo");
	Ammo->DisplayName = FText::FromString(TEXT("Rounds"));
	Ammo->MaxStackSize = 999;
	UDCItemDefinition* Dressing = NewObject<UDCItemDefinition>();
	Dressing->ItemId = TEXT("test.crate_dressing");
	Dressing->MaxStackSize = 10;
	UDCItemDefinition* Wiring = NewObject<UDCItemDefinition>();
	Wiring->ItemId = TEXT("test.crate_wiring");
	Wiring->MaxStackSize = 50;
	for (UDCItemDefinition* Item : { Ammo, Dressing, Wiring })
	{
		Item->AddToRoot();
	}
	ON_SCOPE_EXIT
	{
		for (UDCItemDefinition* Item : { Ammo, Dressing, Wiring })
		{
			Item->ItemId = NAME_None;
			Item->RemoveFromRoot();
		}
		UGameplayStatics::DeleteGameInSlot(Slot, 0);
	};

	// Placed like the map script does it: persistent id and authored contents, then BeginPlay.
	auto SpawnContainer = [&](const FDCTestWorld& World, FName Id)
	{
		ADCLootContainer* Container = World.Get()->SpawnActorDeferred<ADCLootContainer>(
			ADCLootContainer::StaticClass(), FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		Container->FindComponentByClass<UDCPersistentIdComponent>()->SetPersistentId(Id);
		Container->GetInventoryComponent()->AddItem(Ammo, 12);
		Container->GetInventoryComponent()->AddItem(Dressing, 1);
		Container->GetInventoryComponent()->AddItem(Wiring, 3);
		Container->FinishSpawning(FTransform::Identity);
		return Container;
	};

	UDCSaveGame* Save = NewObject<UDCSaveGame>();
	{
		FDCTestWorld World;
		AActor* Player = World.SpawnActor();
		UDCInventoryComponent* Pack = FDCTestWorld::AddComponent<UDCInventoryComponent>(Player);
		ADCLootContainer* Locker = SpawnContainer(World, TEXT("test.locker"));
		ADCLootContainer* Cache = SpawnContainer(World, TEXT("test.cache"));

		TestFalse(TEXT("Only actors with an inventory can loot"), IDCInteractable::Execute_CanInteract(Locker, World.SpawnActor()));
		TestTrue(TEXT("Player can loot"), IDCInteractable::Execute_CanInteract(Locker, Player));
		const FDCInteractionPrompt Prompt = IDCInteractable::Execute_GetInteractionPrompt(Locker, Player);
		TestEqual(TEXT("Prompt names the next stack"), Prompt.TargetName.ToString(), FString(TEXT("Rounds (12) from Container")));

		// Partial loot: one stack per use, oldest first.
		IDCInteractable::Execute_Interact(Locker, Player);
		TestEqual(TEXT("Took the rounds"), Pack->GetQuantity(Ammo), 12);
		TestEqual(TEXT("Two stacks left"), Locker->GetInventoryComponent()->GetStacks().Num(), 2);
		TestEqual(TEXT("Rounds gone from the locker"), Locker->GetInventoryComponent()->GetQuantity(Ammo), 0);

		// Full loot.
		for (int32 Use = 0; Use < 3; ++Use)
		{
			IDCInteractable::Execute_Interact(Cache, Player);
		}
		TestTrue(TEXT("Cache empty"), Cache->IsEmpty());
		TestEqual(TEXT("Player has both rounds stacks"), Pack->GetQuantity(Ammo), 24);
		TestEqual(TEXT("Player has the cache's wiring"), Pack->GetQuantity(Wiring), 3);
		TestTrue(TEXT("Empty container still answers"), IDCInteractable::Execute_CanInteract(Cache, Player));
		TestEqual(TEXT("Empty prompt"), IDCInteractable::Execute_GetInteractionPrompt(Cache, Player).TargetName.ToString(), FString(TEXT("Container (empty)")));
		IDCInteractable::Execute_Interact(Cache, Player);
		TestEqual(TEXT("Searching an empty container gives nothing"), Pack->GetQuantity(Ammo), 24);

		UDCSaveSubsystem::CaptureWorld(Save, World.Get());
	}

	TestTrue(TEXT("Wrote slot"), UGameplayStatics::SaveGameToSlot(Save, Slot, 0));
	const UDCSaveGame* Loaded = Cast<UDCSaveGame>(UGameplayStatics::LoadGameFromSlot(Slot, 0));
	if (!TestNotNull(TEXT("Read slot"), Loaded))
	{
		return false;
	}

	// A fresh map (F9 reopens it): every container starts with its authored contents, then the save applies.
	{
		FDCTestWorld World;
		ADCLootContainer* Locker = SpawnContainer(World, TEXT("test.locker"));
		ADCLootContainer* Cache = SpawnContainer(World, TEXT("test.cache"));
		ADCLootContainer* AddedLater = SpawnContainer(World, TEXT("test.added_later"));
		UDCSaveSubsystem::ApplyWorld(Loaded, World.Get());

		const UDCInventoryComponent* LockerInventory = Locker->GetInventoryComponent();
		TestEqual(TEXT("Partial: exactly two stacks left"), LockerInventory->GetStacks().Num(), 2);
		TestEqual(TEXT("Partial: rounds stay taken"), LockerInventory->GetQuantity(Ammo), 0);
		TestEqual(TEXT("Partial: dressing left"), LockerInventory->GetQuantity(Dressing), 1);
		TestEqual(TEXT("Partial: wiring left"), LockerInventory->GetQuantity(Wiring), 3);
		TestTrue(TEXT("Full: stays empty"), Cache->IsEmpty());
		TestEqual(TEXT("Container newer than the save keeps its contents"), AddedLater->GetInventoryComponent()->GetStacks().Num(), 3);
	}
	return true;
}

#endif
