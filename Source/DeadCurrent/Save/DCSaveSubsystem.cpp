#include "Save/DCSaveSubsystem.h"
#include "Character/DCPlayerCharacter.h"
#include "Combat/DCFirearm.h"
#include "Combat/DCHealthComponent.h"
#include "DeadCurrent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Kismet/GameplayStatics.h"
#include "Quest/DCQuestComponent.h"
#include "Quest/DCQuestDefinition.h"
#include "Save/DCPersistent.h"
#include "Save/DCPersistentRegistry.h"
#include "Save/DCSaveGame.h"
#include "UI/DCHUD.h"

const FString UDCSaveSubsystem::SlotName = TEXT("DeadCurrent");

void UDCSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	static const TCHAR* ItemPaths[] = {
		TEXT("/Game/Items/DA_Item_Pistol.DA_Item_Pistol"),
		TEXT("/Game/Items/DA_Item_Ammo9mm.DA_Item_Ammo9mm"),
		TEXT("/Game/Items/DA_Item_FieldDressing.DA_Item_FieldDressing"),
		TEXT("/Game/Items/DA_Item_SalvagedWiring.DA_Item_SalvagedWiring"),
		TEXT("/Game/Items/DA_Item_RadioCoil.DA_Item_RadioCoil"),
	};
	for (const TCHAR* Path : ItemPaths)
	{
		LoadObject<UDCItemDefinition>(nullptr, Path);
	}

	LoadObject<UDCQuestDefinition>(nullptr, TEXT("/Game/Quests/DA_Quest_ShoreWatch.DA_Quest_ShoreWatch"));
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

	UDCSaveGame* Save = Cast<UDCSaveGame>(UGameplayStatics::CreateSaveGameObject(UDCSaveGame::StaticClass()));
	Save->MapName = World->GetMapName();
	CapturePlayer(Save, Player);
	CaptureWorld(Save, World);

	const bool bOk = UGameplayStatics::SaveGameToSlot(Save, SlotName, 0);
	ADCHUD::ShowMessageFor(Player, bOk
		? NSLOCTEXT("DCSave", "Saved", "Saved.")
		: NSLOCTEXT("DCSave", "SaveFailed", "Save failed."), 2.0f);
	UE_LOG(LogDeadCurrent, Log, TEXT("[DCSAVE] SaveCurrentGame %s"), bOk ? TEXT("ok") : TEXT("failed"));
	return bOk;
}

bool UDCSaveSubsystem::LoadCurrentGame()
{
	UWorld* World = GetWorld();
	ADCPlayerCharacter* Player = World
		? Cast<ADCPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0))
		: nullptr;
	if (!World || !Player)
	{
		return false;
	}

	UDCSaveGame* Save = Cast<UDCSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!Save)
	{
		ADCHUD::ShowMessageFor(Player, NSLOCTEXT("DCSave", "NoSave", "No save."), 2.0f);
		return false;
	}

	ApplyWorld(Save, World);
	ApplyPlayer(Save, Player);
	ADCHUD::ShowMessageFor(Player, NSLOCTEXT("DCSave", "Loaded", "Loaded."), 2.0f);
	UE_LOG(LogDeadCurrent, Log, TEXT("[DCSAVE] LoadCurrentGame ok (%d world actors)"), Save->WorldActors.Num());
	return true;
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

	if (const UDCQuestComponent* Quests = Player->GetQuestComponent())
	{
		Quests->CaptureState(Save->Quests, Save->WorldFlags);
	}
}

void UDCSaveSubsystem::ApplyPlayer(UDCSaveGame* Save, ADCPlayerCharacter* Player) const
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

	if (UDCQuestComponent* Quests = Player->GetQuestComponent())
	{
		Quests->ReplaceFromSaved(Save->Quests, Save->WorldFlags);
	}

	Player->EndSaveRestore();
}

void UDCSaveSubsystem::CaptureWorld(UDCSaveGame* Save, UWorld* World) const
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
	}
}

void UDCSaveSubsystem::ApplyWorld(UDCSaveGame* Save, UWorld* World) const
{
	UDCPersistentRegistry* Registry = World->GetSubsystem<UDCPersistentRegistry>();
	if (!Registry)
	{
		return;
	}

	TSet<FName> SavedIds;
	for (const FDCPersistentActorState& State : Save->WorldActors)
	{
		SavedIds.Add(State.PersistentId);
		if (AActor* Actor = Registry->FindActor(State.PersistentId))
		{
			if (Actor->Implements<UDCPersistent>())
			{
				IDCPersistent::Execute_ApplyPersistentState(Actor, State);
			}
		}
	}

	const TArray<FName> LiveIds = Registry->GetRegisteredIds();
	for (const FName Id : LiveIds)
	{
		if (SavedIds.Contains(Id))
		{
			continue;
		}

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
