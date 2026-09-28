#include "AI/DCFriendlyNPC.h"
#include "AI/DCScavengerCharacter.h"
#include "Character/DCPlayerCharacter.h"
#include "Combat/DCHealthComponent.h"
#include "Dialogue/DCDialogueComponent.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "Interaction/DCInteractable.h"
#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Kismet/GameplayStatics.h"
#include "Quest/DCQuestComponent.h"
#include "Save/DCPersistentRegistry.h"
#include "Save/DCSaveSubsystem.h"
#include "Tests/AutomationCommon.h"
#include "UI/DCHUD.h"
#include "UnrealClient.h"
#include "World/DCInspectableActor.h"
#include "World/DCWorldStateSubsystem.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 *  Plays Shore Watch in the real Lvl_Boathouse (game context: run with -game, see Tools/RunTests.bat -map):
 *  placed actors, real interactions, and real F9 loads that reopen the map. Uses a scratch save slot.
 */
namespace DCBoathouseTest
{
	const TCHAR* MapPath = TEXT("/Game/Maps/Lvl_Boathouse");
	const TCHAR* TestSlot = TEXT("DeadCurrent_MapTest");
	const FName Quest = TEXT("shore.watch");

	UWorld* GameWorld()
	{
		return AutomationCommon::GetAnyGameWorld();
	}

	ADCPlayerCharacter* Player()
	{
		UWorld* World = GameWorld();
		return World ? Cast<ADCPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0)) : nullptr;
	}

	AActor* Find(FName PersistentId)
	{
		UWorld* World = GameWorld();
		const UDCPersistentRegistry* Registry = World ? World->GetSubsystem<UDCPersistentRegistry>() : nullptr;
		return Registry ? Registry->FindActor(PersistentId) : nullptr;
	}

	UDCSaveSubsystem* Saves()
	{
		UWorld* World = GameWorld();
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		return GI ? GI->GetSubsystem<UDCSaveSubsystem>() : nullptr;
	}

	UDCWorldStateSubsystem* WorldState()
	{
		return GameWorld() ? GameWorld()->GetSubsystem<UDCWorldStateSubsystem>() : nullptr;
	}

	/** The relay rig at the scavenger camp (inspectables have no persistent id). */
	ADCInspectableActor* RelayRig()
	{
		ADCInspectableActor* Best = nullptr;
		double BestDist = 150.0;
		for (TActorIterator<ADCInspectableActor> It(GameWorld()); It; ++It)
		{
			const double Dist = FVector::Dist2D(It->GetActorLocation(), FVector(2480.0, -220.0, 0.0));
			if (Dist < BestDist)
			{
				Best = *It;
				BestDist = Dist;
			}
		}
		return Best;
	}

	bool Say(const FString& Text)
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

	FName TalkToMara()
	{
		ADCPlayerCharacter* P = Player();
		AActor* Mara = Find(TEXT("boat.mara"));
		if (!P || !Mara)
		{
			return NAME_None;
		}
		P->GetDialogueComponent()->EndDialogue();
		IDCInteractable::Execute_Interact(Mara, P);
		return P->GetDialogueComponent()->GetCurrentNodeId();
	}

	int32 Count(FName ItemId)
	{
		return Player() ? Player()->GetInventoryComponent()->GetQuantityByItemId(ItemId) : -1;
	}

	FName Stage()
	{
		return Player() ? Player()->GetQuestComponent()->GetStage(Quest) : NAME_None;
	}

	/** Waits until a different world than Previous has begun play with a player in it. */
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

	/** Presses F9: remembers the current world, loads, then waits for the reopened map. */
	void QueueLoad(FAutomationTestBase* Test)
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

	void QueueFreshMap()
	{
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([]()
		{
			if (UDCSaveSubsystem* S = Saves())
			{
				S->SetSlotName(TestSlot);
			}
			UGameplayStatics::DeleteGameInSlot(TestSlot, 0);
			GEngine->Exec(GameWorld(), *FString::Printf(TEXT("Open %s"), MapPath));
			return true;
		}));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.5f));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForMapToLoadCommand());
		ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.5f));
	}

	void QueueCleanup()
	{
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([]()
		{
			UGameplayStatics::DeleteGameInSlot(TestSlot, 0);
			if (UDCSaveSubsystem* S = Saves())
			{
				S->SetSlotName(UDCSaveSubsystem::DefaultSlotName);
			}
			return true;
		}));
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCBoathouseCoilRouteTest, "DeadCurrent.Map.Boathouse.CoilRoute",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCBoathouseCoilRouteTest::RunTest(const FString& Parameters)
{
	using namespace DCBoathouseTest;
	QueueFreshMap();

	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!TestNotNull(TEXT("Player"), Player()) || !TestNotNull(TEXT("Relay rig placed"), RelayRig())
			|| !TestNotNull(TEXT("Mara placed"), Find(TEXT("boat.mara"))) || !TestNotNull(TEXT("Coil placed"), Find(TEXT("boat.pickup_coil"))))
		{
			return true;
		}

		// Clue from the real relay rig variant.
		IDCInteractable::Execute_Interact(RelayRig(), Player());
		TestTrue(TEXT("Rig clue flag"), WorldState()->HasFlag(TEXT("shore.relay_inspected")));

		// Accept from Mara.
		TestEqual(TEXT("Greeting"), TalkToMara(), FName(TEXT("greeting")));
		TestTrue(TEXT("Ask"), Say(TEXT("You keep looking toward his camp.")));
		TestTrue(TEXT("Accept quietly"), Say(TEXT("I'll pull the coil out of his rig. No shooting.")));
		Say(TEXT("Goodbye."));
		TestEqual(TEXT("Accepted"), Stage(), FName(TEXT("accepted")));

		// Take the coil through the real pickup.
		IDCInteractable::Execute_Interact(Find(TEXT("boat.pickup_coil")), Player());
		TestEqual(TEXT("Coil in inventory"), Count(TEXT("radio_coil")), 1);
		TestEqual(TEXT("Quest advanced by pickup"), Stage(), FName(TEXT("return_coil")));

		// With rendering (not -nullrhi), capture the objective line and quest journal for review:
		// Saved/Screenshots/<platform>/DC_QuestHUD.png
		if (FApp::CanEverRender())
		{
			const APlayerController* PC = Cast<APlayerController>(Player()->GetController());
			if (ADCHUD* HUD = PC ? PC->GetHUD<ADCHUD>() : nullptr)
			{
				HUD->ToggleInventory();
			}
			FScreenshotRequest::RequestScreenshot(TEXT("DC_QuestHUD"), true, false);
		}

		TestTrue(TEXT("Save (ready to turn in)"), Saves()->SaveCurrentGame());
		return true;
	}));

	// Give a rendered run time to draw the HUD before the screenshot frame.
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		// Diverge after the save: hand the coil over.
		TestEqual(TEXT("Turn-in node"), TalkToMara(), FName(TEXT("turnin_coil")));
		TestTrue(TEXT("Hand over"), Say(TEXT("Here. It's yours.")));
		TestEqual(TEXT("Done"), Stage(), FName(TEXT("done_coil")));
		return true;
	}));

	// F9: the reopened map must be back at "ready to turn in", coil in hand, pickup gone.
	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!TestNotNull(TEXT("Player after load"), Player()))
		{
			return true;
		}
		TestEqual(TEXT("Load: stage"), Stage(), FName(TEXT("return_coil")));
		TestEqual(TEXT("Load: coil"), Count(TEXT("radio_coil")), 1);
		TestEqual(TEXT("Load: no dressings yet"), Count(TEXT("field_dressing")), 0);
		TestNull(TEXT("Load: coil pickup stays taken"), Find(TEXT("boat.pickup_coil")));
		TestTrue(TEXT("Load: clue flag"), WorldState()->HasFlag(TEXT("shore.relay_inspected")));
		TestFalse(TEXT("Load: outcome flag not set"), WorldState()->HasFlag(TEXT("shore.relay_recovered")));

		TestEqual(TEXT("Load: turn-in node"), TalkToMara(), FName(TEXT("turnin_coil")));
		TestTrue(TEXT("Load: hand over"), Say(TEXT("Here. It's yours.")));
		TestEqual(TEXT("Coil outcome"), Stage(), FName(TEXT("done_coil")));
		TestEqual(TEXT("Dressings"), Count(TEXT("field_dressing")), 2);
		const ADCScavengerCharacter* Scav = Cast<ADCScavengerCharacter>(Find(TEXT("boat.scavenger")));
		TestTrue(TEXT("Scavenger alive on the coil route"), Scav && !Scav->GetHealthComponent()->IsDead());
		TestTrue(TEXT("Save (complete)"), Saves()->SaveCurrentGame());
		return true;
	}));

	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		TestEqual(TEXT("Reload: complete"), Stage(), FName(TEXT("done_coil")));
		TestTrue(TEXT("Reload: outcome flag"), WorldState() && WorldState()->HasFlag(TEXT("shore.relay_recovered")));
		TestEqual(TEXT("Reload: coil gone"), Count(TEXT("radio_coil")), 0);
		TestEqual(TEXT("Reload: epilogue"), TalkToMara(), FName(TEXT("done_coil")));
		return true;
	}));

	QueueCleanup();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCBoathouseCombatRouteTest, "DeadCurrent.Map.Boathouse.CombatRoute",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCBoathouseCombatRouteTest::RunTest(const FString& Parameters)
{
	using namespace DCBoathouseTest;
	QueueFreshMap();

	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		ADCScavengerCharacter* Scav = Cast<ADCScavengerCharacter>(Find(TEXT("boat.scavenger")));
		if (!TestNotNull(TEXT("Player"), Player()) || !TestNotNull(TEXT("Scavenger placed"), Scav))
		{
			return true;
		}

		TestTrue(TEXT("Save before the quest"), Saves()->SaveCurrentGame());

		TalkToMara();
		Say(TEXT("You keep looking toward his camp."));
		Say(TEXT("I'll put him down."));
		FDCDamageInfo Damage;
		Damage.Amount = 1000.0f;
		UDCHealthComponent::ApplyDamageToActor(Scav, Damage);
		TestEqual(TEXT("First kill advanced quest"), Stage(), FName(TEXT("return_killed")));
		return true;
	}));

	// Loading the pre-quest save must bring the scavenger back and forget the quest.
	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		ADCScavengerCharacter* Scav = Cast<ADCScavengerCharacter>(Find(TEXT("boat.scavenger")));
		if (!TestNotNull(TEXT("Scavenger after pre-quest load"), Scav) || !TestNotNull(TEXT("Player after pre-quest load"), Player()))
		{
			return true;
		}
		TestFalse(TEXT("Pre-quest load: scavenger alive"), Scav->GetHealthComponent()->IsDead());
		TestFalse(TEXT("Pre-quest load: quest not started"), Player()->GetQuestComponent()->HasQuest(Quest));
		TestEqual(TEXT("Pre-quest load: greeting"), TalkToMara(), FName(TEXT("greeting")));
		Player()->GetDialogueComponent()->EndDialogue();

		TalkToMara();
		Say(TEXT("You keep looking toward his camp."));
		TestTrue(TEXT("Accept"), Say(TEXT("I'll put him down.")));
		Say(TEXT("Goodbye."));

		// Kill him through the real damage path.
		FDCDamageInfo Damage;
		Damage.Amount = 1000.0f;
		Damage.Instigator = Player();
		UDCHealthComponent::ApplyDamageToActor(Scav, Damage);
		TestTrue(TEXT("Scavenger dead"), Scav->GetHealthComponent()->IsDead());
		TestEqual(TEXT("Kill advanced quest"), Stage(), FName(TEXT("return_killed")));

		// Loot one stack so the save carries a partly looted corpse.
		IDCInteractable::Execute_Interact(Scav, Player());
		TestTrue(TEXT("Save (ready to turn in)"), Saves()->SaveCurrentGame());
		return true;
	}));

	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		const ADCScavengerCharacter* Scav = Cast<ADCScavengerCharacter>(Find(TEXT("boat.scavenger")));
		if (!TestNotNull(TEXT("Scavenger after load"), Scav) || !TestNotNull(TEXT("Player after load"), Player()))
		{
			return true;
		}
		TestTrue(TEXT("Load: scavenger dead (health)"), Scav->GetHealthComponent()->IsDead());
		TestTrue(TEXT("Load: corpse still lootable"), IDCInteractable::Execute_CanInteract(const_cast<ADCScavengerCharacter*>(Scav), Player()));
		TestEqual(TEXT("Load: corpse has two stacks left"), Scav->GetInventoryComponent()->GetStacks().Num(), 2);
		TestEqual(TEXT("Load: stage"), Stage(), FName(TEXT("return_killed")));

		const int32 AmmoBefore = Count(TEXT("ammo_9mm"));
		TestEqual(TEXT("Load: turn-in node"), TalkToMara(), FName(TEXT("turnin_kill")));
		TestTrue(TEXT("Turn in"), Say(TEXT("He's dead. His relay has no one to tend it.")));
		TestEqual(TEXT("Kill outcome"), Stage(), FName(TEXT("done_killed")));
		TestEqual(TEXT("Ammo reward"), Count(TEXT("ammo_9mm")) - AmmoBefore, 24);
		TestTrue(TEXT("Save (complete)"), Saves()->SaveCurrentGame());
		return true;
	}));

	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		TestEqual(TEXT("Reload: complete"), Stage(), FName(TEXT("done_killed")));
		TestTrue(TEXT("Reload: outcome flag"), WorldState() && WorldState()->HasFlag(TEXT("shore.path_cleared")));
		TestEqual(TEXT("Reload: epilogue"), TalkToMara(), FName(TEXT("done_killed")));
		const ADCScavengerCharacter* Scav = Cast<ADCScavengerCharacter>(Find(TEXT("boat.scavenger")));
		TestTrue(TEXT("Reload: scavenger still dead"), Scav && Scav->GetHealthComponent()->IsDead());
		return true;
	}));

	QueueCleanup();
	return true;
}

#endif
