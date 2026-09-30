#pragma once

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Character/DCPlayerCharacter.h"
#include "Combat/DCHealthComponent.h"
#include "Dialogue/DCDialogueComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Interaction/DCInteractable.h"
#include "Inventory/DCInventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Quest/DCQuestComponent.h"
#include "Save/DCPersistentRegistry.h"
#include "Save/DCSaveSubsystem.h"
#include "Tests/AutomationCommon.h"
#include "UI/DCHUD.h"
#include "World/DCInspectableActor.h"
#include "World/DCWorldStateSubsystem.h"

/**
 *  Helpers shared by the in-map tests (game context, `DeadCurrent.Map.*`: one real map, a scratch save slot). They
 *  hold nothing map-specific: the map path and the scratch slot are arguments, and ids, names, and quests are
 *  arguments too. A map's own coordinates, persistent ids, and actor lookups stay in that map's test file.
 *
 *  Extracted from DCBoathouseMapTest.cpp in Phase 6 (VS-02). A new map's tests start from this header:
 *
 *      namespace DCSombreHarborTest
 *      {
 *          const TCHAR* MapPath = TEXT("/Game/Maps/Lvl_PointeSombre");
 *          const TCHAR* TestSlot = TEXT("DeadCurrent_SombreHarborTest");     // one scratch slot per test file
 *          using DCMapTest::Player; using DCMapTest::Find; using DCMapTest::Say; ...
 *      }
 *      ...RunTest: DCMapTest::QueueFreshMap(MapPath, TestSlot);  ...steps...;  DCMapTest::QueueCleanup(TestSlot);
 *
 *  Every helper reads the live world on each call, so one survives an F9 (which reopens the map).
 */
namespace DCMapTest
{
	inline UWorld* GameWorld()
	{
		return AutomationCommon::GetAnyGameWorld();
	}

	inline ADCPlayerCharacter* Player()
	{
		UWorld* World = GameWorld();
		return World ? Cast<ADCPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0)) : nullptr;
	}

	/** The actor registered under a persistent id (`boat.mara`, `sombre.odette`), or null. */
	inline AActor* Find(FName PersistentId)
	{
		UWorld* World = GameWorld();
		const UDCPersistentRegistry* Registry = World ? World->GetSubsystem<UDCPersistentRegistry>() : nullptr;
		return Registry ? Registry->FindActor(PersistentId) : nullptr;
	}

	inline UDCSaveSubsystem* Saves()
	{
		UWorld* World = GameWorld();
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		return GI ? GI->GetSubsystem<UDCSaveSubsystem>() : nullptr;
	}

	inline UDCWorldStateSubsystem* WorldState()
	{
		return GameWorld() ? GameWorld()->GetSubsystem<UDCWorldStateSubsystem>() : nullptr;
	}

	/** Makes the player interact with an actor (an inspectable, a pickup, a container, a portal, an NPC). */
	inline void Use(AActor* Target)
	{
		if (Target && Player())
		{
			IDCInteractable::Execute_Interact(Target, Player());
		}
	}

	/** An inspectable by its display name (inspectables have no persistent id), or null. */
	inline ADCInspectableActor* Inspectable(const TCHAR* DisplayName)
	{
		for (TActorIterator<ADCInspectableActor> It(GameWorld()); It; ++It)
		{
			if (It->GetDisplayName().ToString() == DisplayName)
			{
				return *It;
			}
		}
		return nullptr;
	}

	inline void Use(const TCHAR* InspectableName) { Use(Inspectable(InspectableName)); }

	/** Nearest actor of class T whose bounds center (greybox boxes pivot at a corner) is within MaxDistance. */
	template <class T>
	T* Nearest(const FVector& Where, double MaxDistance = 400.0)
	{
		T* Best = nullptr;
		for (TActorIterator<T> It(GameWorld()); It; ++It)
		{
			FVector Center, Extent;
			It->GetActorBounds(false, Center, Extent);
			const double Dist = FVector::Dist2D(Center, Where);
			if (Dist < MaxDistance)
			{
				Best = *It;
				MaxDistance = Dist;
			}
		}
		return Best;
	}

	/** Puts the player at Where (ending any conversation first). */
	inline void Teleport(const FVector& Where)
	{
		if (ADCPlayerCharacter* P = Player())
		{
			P->GetDialogueComponent()->EndDialogue();
			P->SetActorLocation(Where, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}

	/** Starts the conversation of the NPC registered under the id (ending any current one) and returns its entry node. */
	inline FName TalkTo(FName PersistentId)
	{
		ADCPlayerCharacter* P = Player();
		AActor* Npc = Find(PersistentId);
		if (!P || !Npc)
		{
			return NAME_None;
		}
		P->GetDialogueComponent()->EndDialogue();
		IDCInteractable::Execute_Interact(Npc, P);
		return P->GetDialogueComponent()->GetCurrentNodeId();
	}

	/** Picks the visible reply with exactly this text in the current conversation. False if there is none. */
	inline bool Say(const FString& Text)
	{
		UDCDialogueComponent* Dialogue = Player() ? Player()->GetDialogueComponent() : nullptr;
		const FDCDialogueNode* Node = Dialogue ? Dialogue->GetCurrentNode() : nullptr;
		if (!Node)
		{
			return false;
		}
		const TArray<int32> Visible = Dialogue->GetVisibleChoiceIndices();
		for (int32 Index = 0; Index < Visible.Num(); ++Index)
		{
			if (Node->Choices[Visible[Index]].Text.ToString() == Text)
			{
				return Dialogue->SelectChoice(Index);
			}
		}
		return false;
	}

	/** The replies the player can see right now, in order (failed build checks are hidden, as in play). */
	inline TArray<FString> VisibleChoiceTexts()
	{
		TArray<FString> Texts;
		UDCDialogueComponent* Dialogue = Player() ? Player()->GetDialogueComponent() : nullptr;
		if (const FDCDialogueNode* Node = Dialogue ? Dialogue->GetCurrentNode() : nullptr)
		{
			for (const int32 Index : Dialogue->GetVisibleChoiceIndices())
			{
				Texts.Add(Node->Choices[Index].Text.ToString());
			}
		}
		return Texts;
	}

	inline int32 Count(FName ItemId)
	{
		return Player() ? Player()->GetInventoryComponent()->GetQuantityByItemId(ItemId) : -1;
	}

	/** The player's current stage in a quest (NAME_None when not started). */
	inline FName Stage(FName QuestId)
	{
		return Player() ? Player()->GetQuestComponent()->GetStage(QuestId) : NAME_None;
	}

	inline float Health() { return Player() ? Player()->GetHealthComponent()->GetHealth() : -1.0f; }

	inline ADCHUD* HUD()
	{
		const APlayerController* PC = Player() ? Cast<APlayerController>(Player()->GetController()) : nullptr;
		return PC ? PC->GetHUD<ADCHUD>() : nullptr;
	}

	inline FString Message() { return HUD() ? HUD()->GetActiveMessage().ToString() : FString(); }

	inline FString Banner() { return HUD() ? HUD()->GetActiveBannerSubtitle().ToString() : FString(); }

	/** Points save and load at another slot (a test's scratch slot, or a second one to keep two saves apart). */
	inline void SwitchSlot(const FString& Slot)
	{
		if (UDCSaveSubsystem* S = Saves())
		{
			S->SetSlotName(Slot);
		}
	}

	/** Waits until a different world than Previous has begun play with a player in it (30 s at most). */
	class FWaitForReload : public IAutomationLatentCommand
	{
	public:
		explicit FWaitForReload(TSharedRef<TWeakObjectPtr<UWorld>> InPrevious) : Previous(InPrevious) {}

		virtual bool Update() override
		{
			UWorld* World = GameWorld();
			if (!World || World == Previous->Get() || !World->HasBegunPlay() || !Player())
			{
				return FPlatformTime::Seconds() - StartTime > 30.0;
			}
			return true;
		}

	private:
		TSharedRef<TWeakObjectPtr<UWorld>> Previous;
	};

	/** Presses F9: remembers the current world, loads the current slot, then waits for the reopened map. */
	inline void QueueLoad(FAutomationTestBase* Test)
	{
		TSharedRef<TWeakObjectPtr<UWorld>> Previous = MakeShared<TWeakObjectPtr<UWorld>>();
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([Test, Previous]()
		{
			*Previous = GameWorld();
			Test->TestTrue(TEXT("Load requested"), Saves() && Saves()->LoadCurrentGame());
			return true;
		}));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForReload(Previous));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.5f));
	}

	/** Switches to the scratch slot (emptied), opens the map fresh, and waits until it has loaded. */
	inline void QueueFreshMap(const TCHAR* MapPath, const TCHAR* ScratchSlot)
	{
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([MapPath, ScratchSlot]()
		{
			if (UDCSaveSubsystem* S = Saves())
			{
				S->SetSlotName(ScratchSlot);
			}
			UGameplayStatics::DeleteGameInSlot(ScratchSlot, 0);
			GEngine->Exec(GameWorld(), *FString::Printf(TEXT("Open %s"), MapPath));
			return true;
		}));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.5f));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForMapToLoadCommand());
		ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.5f));
	}

	/** Deletes the scratch slot and points save and load back at the player's own slot. Queue it last. */
	inline void QueueCleanup(const TCHAR* ScratchSlot)
	{
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([ScratchSlot]()
		{
			UGameplayStatics::DeleteGameInSlot(ScratchSlot, 0);
			if (UDCSaveSubsystem* S = Saves())
			{
				S->SetSlotName(UDCSaveSubsystem::DefaultSlotName);
			}
			return true;
		}));
	}

	/** Polls Condition each frame until it returns true, or fails the test after TimeoutSeconds. */
	class FWaitUntil : public IAutomationLatentCommand
	{
	public:
		FWaitUntil(FAutomationTestBase* InTest, TFunction<bool()> InCondition, FString InWhat, double InTimeoutSeconds)
			: Test(InTest), Condition(MoveTemp(InCondition)), What(MoveTemp(InWhat)), TimeoutSeconds(InTimeoutSeconds) {}

		virtual bool Update() override
		{
			if (Condition())
			{
				return true;
			}
			if (FPlatformTime::Seconds() - StartTime > TimeoutSeconds)
			{
				Test->AddError(FString::Printf(TEXT("Timed out after %.1f s waiting for: %s"), TimeoutSeconds, *What));
				return true;
			}
			return false;
		}

	private:
		FAutomationTestBase* Test;
		TFunction<bool()> Condition;
		FString What;
		double TimeoutSeconds;
	};

	/** Waits for a world condition instead of a fixed delay (a presence change applying, a deferred placement). */
	inline void QueueWaitUntil(FAutomationTestBase* Test, TFunction<bool()> Condition, const TCHAR* What, double TimeoutSeconds = 10.0)
	{
		ADD_LATENT_AUTOMATION_COMMAND(FWaitUntil(Test, MoveTemp(Condition), What, TimeoutSeconds));
	}
}

#endif
