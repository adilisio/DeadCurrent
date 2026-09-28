#pragma once

#include "CoreMinimal.h"
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

	UPROPERTY()
	FString MapName;

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

	UPROPERTY()
	TArray<FDCSavedQuestState> Quests;

	UPROPERTY()
	TArray<FName> WorldFlags;
};
