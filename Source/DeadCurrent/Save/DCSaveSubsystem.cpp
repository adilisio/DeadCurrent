#include "Save/DCSaveSubsystem.h"
#include "Character/DCCharacterProgressionComponent.h"
#include "Character/DCPlayerCharacter.h"
#include "Combat/DCFirearm.h"
#include "Combat/DCHealthComponent.h"
#include "DeadCurrent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "Quest/DCQuestComponent.h"
#include "Save/DCPersistent.h"
#include "Save/DCPersistentRegistry.h"
#include "Save/DCSaveGame.h"
#include "UI/DCHUD.h"
#include "World/DCWorldStateSubsystem.h"

const FString UDCSaveSubsystem::DefaultSlotName = TEXT("DeadCurrent");

/** Map package without the PIE prefix, e.g. /Game/Maps/Lvl_Boathouse. */
static FString DCMapPackage(const UWorld* World)
{
	return World ? UWorld::RemovePIEPrefix(World->GetOutermost()->GetName()) : FString();
}

static FString DCShortMapName(const UWorld* World)
{
	if (!World)
	{
		return FString();
	}

	const FString FromPackage = FPackageName::GetShortName(DCMapPackage(World));
	return FromPackage.IsEmpty() ? World->GetMapName() : FromPackage;
}

static bool DCMapsMatch(const FString& Saved, const UWorld* World)
{
	if (Saved.IsEmpty() || !World)
	{
		return true;
	}

	const FString Current = DCShortMapName(World);
	return Saved.Equals(Current, ESearchCase::IgnoreCase)
		|| Saved.Equals(World->GetMapName(), ESearchCase::IgnoreCase);
}

static void DCAppendFlatInventory(UDCSaveGame* Save, FName Id, const TArray<FDCSavedItemStack>& Stacks)
{
	Save->WorldInvActorIds.Add(Id);
	for (const FDCSavedItemStack& Stack : Stacks)
	{
		Save->WorldInvStackActorIds.Add(Id);
		Save->WorldInvItemIds.Add(Stack.ItemId);
		Save->WorldInvQuantities.Add(Stack.Quantity);
		Save->WorldInvItemPaths.Add(Stack.ItemPath);
	}
}

static bool DCTakeFlatInventory(const UDCSaveGame* Save, FName Id, TArray<FDCSavedItemStack>& OutStacks)
{
	OutStacks.Reset();
	const int32 Num = FMath::Min(Save->WorldInvStackActorIds.Num(),
		FMath::Min(Save->WorldInvItemIds.Num(),
			FMath::Min(Save->WorldInvQuantities.Num(), Save->WorldInvItemPaths.Num())));
	for (int32 Index = 0; Index < Num; ++Index)
	{
		if (Save->WorldInvStackActorIds[Index] != Id)
		{
			continue;
		}

		FDCSavedItemStack Stack;
		Stack.ItemId = Save->WorldInvItemIds[Index];
		Stack.Quantity = Save->WorldInvQuantities[Index];
		Stack.ItemPath = Save->WorldInvItemPaths[Index];
		OutStacks.Add(Stack);
	}

	return Save->WorldInvActorIds.Contains(Id);
}

bool UDCSaveSubsystem::SaveCurrentGame()
{
	UWorld* World = GetWorld();
	ADCPlayerCharacter* Player = World
		? Cast<ADCPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0))
		: nullptr;
	if (!World || !Player)
	{
		return false;
	}

	// A save taken while dead would load into a dead player with no respawn pending.
	if (Player->GetHealthComponent() && Player->GetHealthComponent()->IsDead())
	{
		ADCHUD::ShowMessageFor(Player, NSLOCTEXT("DCSave", "DeadNoSave", "You can't save now."), 2.0f);
		return false;
	}

	UDCSaveGame* Save = Cast<UDCSaveGame>(UGameplayStatics::CreateSaveGameObject(UDCSaveGame::StaticClass()));
	Save->SaveVersion = UDCSaveGame::CurrentVersion;
	Save->MapName = DCShortMapName(World);
	Save->MapPackage = DCMapPackage(World);
	CapturePlayer(Save, Player);
	CaptureWorld(Save, World);

	const bool bOk = UGameplayStatics::SaveGameToSlot(Save, SlotName, 0);
	ADCHUD::ShowMessageFor(Player, bOk
		? NSLOCTEXT("DCSave", "Saved", "Saved.")
		: NSLOCTEXT("DCSave", "SaveFailed", "Save failed."), 2.0f);
	UE_LOG(LogDeadCurrent, Log, TEXT("[DCSAVE] SaveCurrentGame %s (%s, %d quests, %d flags, %d locations)"), bOk ? TEXT("ok") : TEXT("failed"),
		*Save->MapPackage, Save->Quests.Num(), Save->WorldFlags.Num(), Save->DiscoveredLocations.Num());
	return bOk;
}

bool UDCSaveSubsystem::LoadCurrentGame()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	APawn* Player = UGameplayStatics::GetPlayerPawn(World, 0);
	UDCSaveGame* Save = Cast<UDCSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!Save)
	{
		ADCHUD::ShowMessageFor(Player, NSLOCTEXT("DCSave", "NoSave", "No save."), 2.0f);
		return false;
	}

	FString Map = Save->MapPackage.IsEmpty() ? Save->MapName : Save->MapPackage;
	if (Map.IsEmpty())
	{
		Map = DCMapPackage(World);
	}

	// Reopen the map so every actor starts fresh (pickups back, enemies alive), then apply the save.
	PendingLoad = Save;
	UE_LOG(LogDeadCurrent, Log, TEXT("[DCSAVE] LoadCurrentGame: opening %s (save version %d)"), *Map, Save->SaveVersion);
	UGameplayStatics::OpenLevel(World, FName(*Map));
	return true;
}

void UDCSaveSubsystem::ApplyPendingLoad(UWorld* World)
{
	if (!PendingLoad || !World)
	{
		return;
	}

	UDCSaveGame* Save = PendingLoad;
	PendingLoad = nullptr;

	ADCPlayerCharacter* Player = Cast<ADCPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
	if (!Player)
	{
		UE_LOG(LogDeadCurrent, Warning, TEXT("[DCSAVE] ApplyPendingLoad: no player character"));
		return;
	}

	if (!DCMapsMatch(Save->MapName, World))
	{
		ADCHUD::ShowMessageFor(Player, NSLOCTEXT("DCSave", "WrongMap", "Save is for a different map."), 3.0f);
		UE_LOG(LogDeadCurrent, Warning, TEXT("[DCSAVE] ApplyPendingLoad skipped: save map %s, current %s"),
			*Save->MapName, *DCShortMapName(World));
		return;
	}

	// World state first: anything that reacts to where things end up (discovery volumes, world-conditioned
	// hazards) must already see the saved flags and discoveries, so nothing is re-announced on load.
	ApplyWorldState(Save, World->GetSubsystem<UDCWorldStateSubsystem>());
	ApplyWorld(Save, World);
	ApplyPlayer(Save, Player);
	// Everything is in place: presentation that normally changes out of sight (conditional presence) snaps now.
	if (UDCWorldStateSubsystem* WorldState = World->GetSubsystem<UDCWorldStateSubsystem>())
	{
		WorldState->NotifyRestored();
	}
	ADCHUD::ShowMessageFor(Player, NSLOCTEXT("DCSave", "Loaded", "Loaded."), 2.0f);
	UE_LOG(LogDeadCurrent, Log, TEXT("[DCSAVE] load applied (%d world actors, %d quests, %d flags, %d locations)"),
		Save->WorldActors.Num(), Save->Quests.Num(), Save->WorldFlags.Num(), Save->DiscoveredLocations.Num());
}

void UDCSaveSubsystem::CaptureProgress(UDCSaveGame* Save, const UDCQuestComponent* Quests, const UDCWorldStateSubsystem* WorldState)
{
	if (Quests)
	{
		Quests->CaptureState(Save->Quests);
	}
	if (WorldState)
	{
		Save->WorldFlags = WorldState->GetFlags();
		Save->DiscoveredLocations = WorldState->GetDiscoveredLocations();
	}
}

void UDCSaveSubsystem::ApplyProgress(const UDCSaveGame* Save, UDCQuestComponent* Quests, UDCWorldStateSubsystem* WorldState)
{
	ApplyWorldState(Save, WorldState);
	if (Quests)
	{
		Quests->ReplaceFromSaved(Save->Quests);
	}
}

void UDCSaveSubsystem::ApplyWorldState(const UDCSaveGame* Save, UDCWorldStateSubsystem* WorldState)
{
	if (WorldState)
	{
		WorldState->ReplaceFlags(Save->WorldFlags);
		WorldState->ReplaceDiscoveredLocations(Save->DiscoveredLocations);
	}
}

void UDCSaveSubsystem::CapturePlayer(UDCSaveGame* Save, const ADCPlayerCharacter* Player) const
{
	Save->PlayerLocation = Player->GetActorLocation();
	Save->PlayerRotation = Player->GetActorRotation();
	if (const AController* PC = Player->GetController())
	{
		Save->ControlRotation = PC->GetControlRotation();
	}

	if (const UDCHealthComponent* Health = Player->GetHealthComponent())
	{
		Save->PlayerHealth = Health->GetHealth();
		Save->bPlayerDead = Health->IsDead();
	}

	if (const UDCInventoryComponent* Inventory = Player->GetInventoryComponent())
	{
		Inventory->CaptureStacks(Save->PlayerInventory);
	}

	if (const ADCFirearm* Firearm = Player->GetEquippedFirearm())
	{
		if (const UDCItemDefinition* Def = Firearm->GetDefinition())
		{
			Save->EquippedItemId = Def->ItemId;
		}
		Save->MagazineRounds = Firearm->GetRoundsInMagazine();
		Save->bWeaponHolstered = Firearm->IsHolstered();
	}

	CaptureProgress(Save, Player->GetQuestComponent(), UDCWorldStateSubsystem::Get(Player));
	CaptureBuild(Save, Player->GetProgressionComponent());
}

void UDCSaveSubsystem::ApplyPlayer(const UDCSaveGame* Save, ADCPlayerCharacter* Player) const
{
	Player->BeginSaveRestore();

	Player->SetActorLocationAndRotation(Save->PlayerLocation, Save->PlayerRotation, false, nullptr, ETeleportType::TeleportPhysics);
	if (AController* PC = Player->GetController())
	{
		PC->SetControlRotation(Save->ControlRotation);
	}

	if (UDCHealthComponent* Health = Player->GetHealthComponent())
	{
		Health->ApplyLoadedState(Save->PlayerHealth, Save->bPlayerDead);
	}

	if (UDCInventoryComponent* Inventory = Player->GetInventoryComponent())
	{
		Player->ClearEquippedFirearm();
		Inventory->ReplaceFromSaved(Save->PlayerInventory);
	}

	if (ADCFirearm* Firearm = Player->GetEquippedFirearm())
	{
		Firearm->RestoreMagazine(Save->MagazineRounds);
		Firearm->SetHolstered(Save->bWeaponHolstered);
	}

	ApplyProgress(Save, Player->GetQuestComponent(), UDCWorldStateSubsystem::Get(Player));
	ApplyBuild(Save, Player->GetProgressionComponent());

	Player->EndSaveRestore();
}

void UDCSaveSubsystem::CaptureBuild(UDCSaveGame* Save, const UDCCharacterProgressionComponent* Build)
{
	if (Save && Build)
	{
		Build->CaptureState(Save->Attributes, Save->Skills, Save->Perks);
	}
}

void UDCSaveSubsystem::ApplyBuild(const UDCSaveGame* Save, UDCCharacterProgressionComponent* Build)
{
	if (Save && Build)
	{
		Build->RestoreState(Save->Attributes, Save->Skills, Save->Perks);
	}
}

void UDCSaveSubsystem::CaptureWorld(UDCSaveGame* Save, UWorld* World)
{
	const UDCPersistentRegistry* Registry = World->GetSubsystem<UDCPersistentRegistry>();
	if (!Registry)
	{
		return;
	}

	for (const FName Id : Registry->GetRegisteredIds())
	{
		AActor* Actor = Registry->FindActor(Id);
		if (!Actor || !Actor->Implements<UDCPersistent>())
		{
			continue;
		}

		FDCPersistentActorState State;
		IDCPersistent::Execute_CapturePersistentState(Actor, State);
		State.PersistentId = Id;
		Save->WorldActors.Add(State);

		TArray<FDCSavedItemStack> Stacks;
		if (const UDCInventoryComponent* Inventory = Actor->FindComponentByClass<UDCInventoryComponent>())
		{
			Inventory->CaptureStacks(Stacks);
			DCAppendFlatInventory(Save, Id, Stacks);
		}

		FDCSavedActorInventory ActorInventory;
		ActorInventory.PersistentId = Id;
		ActorInventory.Stacks = Stacks;
		Save->ActorInventories.Add(ActorInventory);
		UE_LOG(LogDeadCurrent, Log, TEXT("[DCSAVE] capture %s alive=%d stacks=%d"),
			*Id.ToString(), State.bAlive ? 1 : 0, Stacks.Num());
	}

	Save->RemovedPersistentIds = Registry->GetRemovedIds();
}

/**
 *  Saves before version 4 did not record removals. On Lvl_Boathouse, a pickup that existed when
 *  that save was written and is missing from WorldActors was taken. Pickups added to the map
 *  later are not in this list, so they stay. The coil arrived with the Micro RPG (version 2).
 */
static void DCAddHistoricalTakenPickups(const UDCSaveGame* Save, const TSet<FName>& SavedIds, TArray<FName>& OutRemoved)
{
	if (!Save || Save->SaveVersion >= 4)
	{
		return;
	}

	static const FName Known[] = {
		TEXT("boat.pickup_pistol"),
		TEXT("boat.pickup_ammo"),
		TEXT("boat.pickup_dressing"),
		TEXT("boat.pickup_coil"),
	};
	const int32 Count = Save->SaveVersion >= 2 ? 4 : 3;
	for (int32 Index = 0; Index < Count; ++Index)
	{
		if (!SavedIds.Contains(Known[Index]) && !OutRemoved.Contains(Known[Index]))
		{
			OutRemoved.Add(Known[Index]);
		}
	}
}

void UDCSaveSubsystem::ApplyWorld(const UDCSaveGame* Save, UWorld* World)
{
	UDCPersistentRegistry* Registry = World->GetSubsystem<UDCPersistentRegistry>();
	if (!Registry)
	{
		return;
	}

	TMap<FName, int32> InventoryIndex;
	for (int32 Index = 0; Index < Save->ActorInventories.Num(); ++Index)
	{
		const FName Id = Save->ActorInventories[Index].PersistentId;
		if (!Id.IsNone())
		{
			InventoryIndex.Add(Id, Index);
		}
	}

	TSet<FName> SavedIds;
	for (const FDCPersistentActorState& State : Save->WorldActors)
	{
		SavedIds.Add(State.PersistentId);
		FDCPersistentActorState ToApply = State;
		ToApply.Inventory.Reset();

		if (AActor* Actor = Registry->FindActor(State.PersistentId))
		{
			if (Actor->Implements<UDCPersistent>())
			{
				IDCPersistent::Execute_ApplyPersistentState(Actor, ToApply);
			}

			UDCInventoryComponent* Inventory = Actor->FindComponentByClass<UDCInventoryComponent>();
			if (!Inventory)
			{
				continue;
			}

			TArray<FDCSavedItemStack> Stacks;
			if (DCTakeFlatInventory(Save, State.PersistentId, Stacks))
			{
				Inventory->ReplaceFromSaved(Stacks);
			}
			else if (const int32* InvIndex = InventoryIndex.Find(State.PersistentId))
			{
				Inventory->ReplaceFromSaved(Save->ActorInventories[*InvIndex].Stacks);
			}

			UE_LOG(LogDeadCurrent, Log, TEXT("[DCSAVE] apply %s alive=%d stacks=%d"),
				*State.PersistentId.ToString(), ToApply.bAlive ? 1 : 0, Inventory->GetStacks().Num());
		}
	}

	// Only ids we know were removed. A live actor that is simply absent from the save was not
	// in the world when the save was written, and keeps its authored state.
	TArray<FName> Removed = Save->RemovedPersistentIds;
	DCAddHistoricalTakenPickups(Save, SavedIds, Removed);
	Registry->RestoreRemovedIds(Removed);

	for (const FName Id : Removed)
	{
		AActor* Actor = Registry->FindActor(Id);
		if (!Actor || !Actor->Implements<UDCPersistent>())
		{
			continue;
		}

		FDCPersistentActorState Missing;
		Missing.PersistentId = Id;
		Missing.bExists = false;
		IDCPersistent::Execute_ApplyPersistentState(Actor, Missing);
	}
}
