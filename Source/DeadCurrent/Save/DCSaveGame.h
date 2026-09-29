#pragma once

#include "CoreMinimal.h"
#include "Character/DCProgressionTypes.h"
#include "GameFramework/SaveGame.h"
#include "Save/DCPersistentTypes.h"
#include "DCSaveGame.generated.h"

/**
 *  One save slot. Player plus every persistent world actor that still exists.
 */
UCLASS()
class DEADCURRENT_API UDCSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	/** Written as CurrentVersion. Saves from before versioning load as 0. */
	UPROPERTY()
	int32 SaveVersion = 0;

	/**
	 *  1: first playable. 2: micro RPG (MapPackage, world flags owned by UDCWorldStateSubsystem,
	 *  quest stages from data-driven quests). 3: exploration loop (DiscoveredLocations; loot
	 *  containers use the existing world inventory arrays). 4: RemovedPersistentIds. 5: character
	 *  build (Attributes, Skills, Perks). Older saves still load. Quest stages that no longer
	 *  exist are dropped. Saves before 3 have no discovered locations. Saves before 4 do not
	 *  list removed actors; load treats only the boathouse pickups that existed at that version,
	 *  and were absent from WorldActors, as taken. Saves before 5 load an unspent build.
	 */
	static constexpr int32 CurrentVersion = 5;

	/** Short map name, e.g. Lvl_Boathouse. */
	UPROPERTY()
	FString MapName;

	/** Long package name, e.g. /Game/Maps/Lvl_Boathouse. Load opens this map before applying the save. */
	UPROPERTY()
	FString MapPackage;

	UPROPERTY()
	FVector PlayerLocation = FVector::ZeroVector;

	UPROPERTY()
	FRotator PlayerRotation = FRotator::ZeroRotator;

	UPROPERTY()
	FRotator ControlRotation = FRotator::ZeroRotator;

	UPROPERTY()
	float PlayerHealth = 100.0f;

	UPROPERTY()
	bool bPlayerDead = false;

	UPROPERTY()
	TArray<FDCSavedItemStack> PlayerInventory;

	UPROPERTY()
	FName EquippedItemId;

	UPROPERTY()
	int32 MagazineRounds = 0;

	UPROPERTY()
	bool bWeaponHolstered = false;

	UPROPERTY()
	TArray<FDCPersistentActorState> WorldActors;

	/** Inventories keyed by PersistentId. Nested arrays on WorldActors do not round-trip through USaveGame. */
	UPROPERTY()
	TArray<FDCSavedActorInventory> ActorInventories;

	/**
	 *  World inventories as parallel primitive arrays. Nested TArray<FDCSavedItemStack>
	 *  (and TSoftObjectPtr inside those stacks) can serialize empty through USaveGame.
	 *  WorldInvActorIds lists every actor whose inventory was captured, including empty corpses.
	 */
	UPROPERTY()
	TArray<FName> WorldInvActorIds;

	UPROPERTY()
	TArray<FName> WorldInvStackActorIds;

	UPROPERTY()
	TArray<FName> WorldInvItemIds;

	UPROPERTY()
	TArray<int32> WorldInvQuantities;

	UPROPERTY()
	TArray<FString> WorldInvItemPaths;

	/** Player quest log: quest id + current stage. */
	UPROPERTY()
	TArray<FDCSavedQuestState> Quests;

	/** UDCWorldStateSubsystem flags. */
	UPROPERTY()
	TArray<FName> WorldFlags;

	/** UDCWorldStateSubsystem discovered locations, in discovery order. */
	UPROPERTY()
	TArray<FName> DiscoveredLocations;

	/**
	 *  Persistent ids removed during play (a pickup the player took). Absence from WorldActors
	 *  is not removal: an actor added to the map after the save was written is left as authored.
	 *  Saves older than version 4 leave this empty; see UDCSaveSubsystem::ApplyWorld.
	 */
	UPROPERTY()
	TArray<FName> RemovedPersistentIds;

	/** Character build. Empty means the unspent default, including saves from before version 5. */
	UPROPERTY()
	TArray<FDCSavedRank> Attributes;

	UPROPERTY()
	TArray<FDCSavedRank> Skills;

	/** Owned perk tag names, e.g. Perk.SchematicEye. */
	UPROPERTY()
	TArray<FName> Perks;
};
