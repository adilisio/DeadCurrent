#include "Combat/DCHealthComponent.h"
#include "Core/DCContentSubsystem.h"
#include "Core/DCGameplayRules.h"
#include "Core/DCTestHelpers.h"
#include "Dialogue/DCDialogueAsset.h"
#include "Dialogue/DCDialogueComponent.h"
#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Quest/DCQuestComponent.h"
#include "Quest/DCQuestDefinition.h"
#include "Save/DCPersistentIdComponent.h"
#include "Save/DCSaveGame.h"
#include "Save/DCSaveSubsystem.h"
#include "World/DCInspectableActor.h"
#include "World/DCWorldStateSubsystem.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 *  Plays Shore Watch through the shipped assets (DA_Quest_ShoreWatch, DA_Dialogue_MaraIntro):
 *  both routes, the pre-quest shortcuts, the clue line, and a save round trip at each stage.
 *  The scavenger is a stand-in actor with the real persistent id and a health component.
 */
namespace DCShoreWatchTest
{
	const FName Quest = TEXT("shore.watch");
	const TCHAR* DialoguePath = TEXT("/Game/Dialogue/DA_Dialogue_MaraIntro.DA_Dialogue_MaraIntro");

	struct FShore
	{
		FDCTestWorld World;
		AActor* Player = nullptr;
		UDCInventoryComponent* Inventory = nullptr;
		UDCQuestComponent* Quests = nullptr;
		UDCDialogueComponent* Dialogue = nullptr;
		UDCHealthComponent* Scavenger = nullptr;
		UDCWorldStateSubsystem* WorldState = nullptr;
		const UDCDialogueAsset* Mara = nullptr;
		const UDCItemDefinition* Coil = nullptr;
		const UDCItemDefinition* Ammo = nullptr;
		const UDCItemDefinition* Dressing = nullptr;

		FShore()
		{
			Player = World.SpawnActor();
			Inventory = FDCTestWorld::AddComponent<UDCInventoryComponent>(Player);
			Quests = FDCTestWorld::AddComponent<UDCQuestComponent>(Player);
			Dialogue = FDCTestWorld::AddComponent<UDCDialogueComponent>(Player);

			AActor* Scav = World.SpawnActor();
			FDCTestWorld::AddComponent<UDCPersistentIdComponent>(Scav, [](UDCPersistentIdComponent* C) { C->SetPersistentId(TEXT("boat.scavenger")); });
			Scavenger = FDCTestWorld::AddComponent<UDCHealthComponent>(Scav);

			WorldState = World.Get()->GetSubsystem<UDCWorldStateSubsystem>();
			Mara = LoadObject<UDCDialogueAsset>(nullptr, DialoguePath);
			Coil = UDCItemDefinition::FindByItemId(TEXT("radio_coil"));
			Ammo = UDCItemDefinition::FindByItemId(TEXT("ammo_9mm"));
			Dressing = UDCItemDefinition::FindByItemId(TEXT("field_dressing"));
		}

		bool IsValid() const { return Mara && Coil && Ammo && Dressing && UDCQuestDefinition::FindByQuestId(Quest); }

		FName Talk() const
		{
			Dialogue->EndDialogue();
			Dialogue->StartDialogue(Mara, nullptr);
			return Dialogue->GetCurrentNodeId();
		}

		TArray<FString> VisibleChoices() const
		{
			TArray<FString> Texts;
			if (const FDCDialogueNode* Node = Dialogue->GetCurrentNode())
			{
				for (const int32 Index : Dialogue->GetVisibleChoiceIndices())
				{
					Texts.Add(Node->Choices[Index].Text.ToString());
				}
			}
			return Texts;
		}

		bool Can(const FString& Text) const { return VisibleChoices().Contains(Text); }

		/** Picks a visible reply by its text, like pressing its number key. */
		bool Say(const FString& Text) const
		{
			const int32 Index = VisibleChoices().IndexOfByKey(Text);
			return Index != INDEX_NONE && Dialogue->SelectChoice(Index);
		}

		void KillScavenger() const
		{
			FDCDamageInfo Damage;
			Damage.Amount = Scavenger->GetMaxHealth();
			Scavenger->ApplyDamage(Damage);
		}

		FName Stage() const { return Quests->GetStage(Quest); }

		/** Save progress to a save object and restore it into a fresh shore (scavenger dead if it was). */
		TUniquePtr<FShore> SaveAndReload() const
		{
			UDCSaveGame* Save = NewObject<UDCSaveGame>();
			UDCSaveSubsystem::CaptureProgress(Save, Quests, WorldState);
			TArray<FDCSavedItemStack> Stacks;
			Inventory->CaptureStacks(Stacks);

			TUniquePtr<FShore> Reloaded = MakeUnique<FShore>();
			if (Scavenger->IsDead())
			{
				Reloaded->Scavenger->ApplyLoadedState(0.0f, true);
			}
			Reloaded->Inventory->ReplaceFromSaved(Stacks);
			UDCSaveSubsystem::ApplyProgress(Save, Reloaded->Quests, Reloaded->WorldState);
			return Reloaded;
		}
	};

	void LoadContent()
	{
		TArray<UObject*> Definitions;
		UDCContentSubsystem::LoadAllDefinitions(Definitions);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCShoreWatchCombatTest, "DeadCurrent.Content.ShoreWatch.CombatRoute",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDCShoreWatchCombatTest::RunTest(const FString& Parameters)
{
	using namespace DCShoreWatchTest;
	LoadContent();
	FShore Shore;
	if (!TestTrue(TEXT("Content loaded"), Shore.IsValid()))
	{
		return false;
	}

	// Before the quest.
	TestEqual(TEXT("Greeting"), Shore.Talk(), FName(TEXT("greeting")));
	TestTrue(TEXT("Offer lead visible"), Shore.Can(TEXT("You keep looking toward his camp.")));
	TestTrue(TEXT("Ask"), Shore.Say(TEXT("You keep looking toward his camp.")));
	TestFalse(TEXT("Coil shortcut hidden without coil"), Shore.Can(TEXT("This coil? I already pulled it.")));
	TestTrue(TEXT("Accept"), Shore.Say(TEXT("I'll put him down.")));
	TestEqual(TEXT("Quest accepted"), Shore.Stage(), FName(TEXT("accepted")));
	TestFalse(TEXT("Objective shown"), Shore.Quests->GetObjectiveText().IsEmpty());

	// Save while active, reload: still in progress and still listening for the kill.
	TUniquePtr<FShore> Active = Shore.SaveAndReload();
	TestEqual(TEXT("Reload: accepted"), Active->Stage(), FName(TEXT("accepted")));
	TestEqual(TEXT("Reload: in-progress dialogue"), Active->Talk(), FName(TEXT("inprogress")));
	TestFalse(TEXT("Offer gone once started"), Active->Can(TEXT("You keep looking toward his camp.")));

	// Combat resolves the objective with no quest code in the scavenger.
	Active->KillScavenger();
	TestEqual(TEXT("Kill advances quest"), Active->Stage(), FName(TEXT("return_killed")));

	// Ready to turn in survives a reload.
	TUniquePtr<FShore> Ready = Active->SaveAndReload();
	TestEqual(TEXT("Reload: return_killed"), Ready->Stage(), FName(TEXT("return_killed")));
	TestEqual(TEXT("Turn-in dialogue"), Ready->Talk(), FName(TEXT("turnin_kill")));
	TestTrue(TEXT("Turn in"), Ready->Say(TEXT("He's dead. His relay has no one to tend it.")));
	TestTrue(TEXT("Complete"), Ready->Quests->IsComplete(Quest));
	TestEqual(TEXT("Kill outcome"), Ready->Stage(), FName(TEXT("done_killed")));
	TestEqual(TEXT("Ammo reward"), Ready->Inventory->GetQuantity(Ready->Ammo), 24);
	TestTrue(TEXT("path_cleared flag"), Ready->WorldState->HasFlag(TEXT("shore.path_cleared")));
	TestFalse(TEXT("No relay_recovered flag"), Ready->WorldState->HasFlag(TEXT("shore.relay_recovered")));

	// After completion, and after another reload, Mara remembers how it ended.
	TUniquePtr<FShore> Done = Ready->SaveAndReload();
	TestEqual(TEXT("Done dialogue"), Done->Talk(), FName(TEXT("done_killed")));
	TestTrue(TEXT("Reload keeps outcome flag"), Done->WorldState->HasFlag(TEXT("shore.path_cleared")));
	TestEqual(TEXT("Reward not paid twice"), Done->Inventory->GetQuantity(Done->Ammo), 24);

	// Late coil hand-over after the kill route.
	Done->Dialogue->EndDialogue();
	Done->Inventory->AddItem(Done->Coil, 1);
	Done->Talk();
	TestTrue(TEXT("Late coil offer"), Done->Say(TEXT("I pulled the coil from his relay, too.")));
	TestEqual(TEXT("Coil taken"), Done->Inventory->GetQuantity(Done->Coil), 0);
	TestTrue(TEXT("relay_recovered after late hand-over"), Done->WorldState->HasFlag(TEXT("shore.relay_recovered")));
	TestEqual(TEXT("Outcome unchanged"), Done->Stage(), FName(TEXT("done_killed")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCShoreWatchStealthTest, "DeadCurrent.Content.ShoreWatch.CoilRoute",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDCShoreWatchStealthTest::RunTest(const FString& Parameters)
{
	using namespace DCShoreWatchTest;
	LoadContent();
	FShore Shore;
	if (!TestTrue(TEXT("Content loaded"), Shore.IsValid()))
	{
		return false;
	}

	Shore.Talk();
	Shore.Say(TEXT("You keep looking toward his camp."));
	TestTrue(TEXT("Accept quietly"), Shore.Say(TEXT("I'll pull the coil out of his rig. No shooting.")));
	TestEqual(TEXT("Same quest, same stage"), Shore.Stage(), FName(TEXT("accepted")));

	// Inspecting the live relay is a clue that opens a line with Mara (set here as the rig's consequence would).
	Shore.WorldState->SetFlag(TEXT("shore.relay_inspected"));
	TestEqual(TEXT("In progress"), Shore.Talk(), FName(TEXT("inprogress")));
	TestTrue(TEXT("Clue line visible"), Shore.Say(TEXT("I looked at his relay. It's saying words.")));
	TestEqual(TEXT("Voice node"), Shore.Dialogue->GetCurrentNodeId(), FName(TEXT("voice")));
	Shore.Say(TEXT("I won't."));
	TestTrue(TEXT("Clue remembered"), Shore.WorldState->HasFlag(TEXT("shore.voice_discussed")));
	Shore.Talk();
	TestFalse(TEXT("Clue line gone after telling her"), Shore.Can(TEXT("I looked at his relay. It's saying words.")));
	Shore.Dialogue->EndDialogue();

	// Taking the coil resolves the objective; the scavenger lives.
	Shore.Inventory->AddItem(Shore.Coil, 1);
	TestEqual(TEXT("Coil advances quest"), Shore.Stage(), FName(TEXT("return_coil")));

	TUniquePtr<FShore> Ready = Shore.SaveAndReload();
	TestEqual(TEXT("Reload: return_coil"), Ready->Stage(), FName(TEXT("return_coil")));
	TestEqual(TEXT("Reload: coil kept"), Ready->Inventory->GetQuantity(Ready->Coil), 1);
	TestEqual(TEXT("Coil turn-in dialogue"), Ready->Talk(), FName(TEXT("turnin_coil")));
	TestTrue(TEXT("Hand over"), Ready->Say(TEXT("Here. It's yours.")));
	TestEqual(TEXT("Coil outcome"), Ready->Stage(), FName(TEXT("done_coil")));
	TestEqual(TEXT("Coil removed"), Ready->Inventory->GetQuantity(Ready->Coil), 0);
	TestEqual(TEXT("Dressing reward"), Ready->Inventory->GetQuantity(Ready->Dressing), 2);
	TestEqual(TEXT("No ammo reward"), Ready->Inventory->GetQuantity(Ready->Ammo), 0);
	TestTrue(TEXT("relay_recovered flag"), Ready->WorldState->HasFlag(TEXT("shore.relay_recovered")));
	TestFalse(TEXT("No path_cleared flag"), Ready->WorldState->HasFlag(TEXT("shore.path_cleared")));
	TestFalse(TEXT("Scavenger still alive"), Ready->Scavenger->IsDead());

	TUniquePtr<FShore> Done = Ready->SaveAndReload();
	TestEqual(TEXT("Coil-route epilogue"), Done->Talk(), FName(TEXT("done_coil")));
	TestFalse(TEXT("No kill remark while he lives"), Done->Can(TEXT("He won't be walking anywhere now.")));
	Done->Dialogue->EndDialogue();

	// Killing him later is noticed once, and does not rewrite the outcome.
	Done->KillScavenger();
	Done->Talk();
	TestTrue(TEXT("Kill remark"), Done->Say(TEXT("He won't be walking anywhere now.")));
	TestEqual(TEXT("Outcome kept"), Done->Stage(), FName(TEXT("done_coil")));
	Done->Talk();
	TestFalse(TEXT("Kill remark only once"), Done->Can(TEXT("He won't be walking anywhere now.")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCShoreWatchShortcutTest, "DeadCurrent.Content.ShoreWatch.Shortcuts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDCShoreWatchShortcutTest::RunTest(const FString& Parameters)
{
	using namespace DCShoreWatchTest;
	LoadContent();

	// Scavenger killed on the way to Mara (the first-playable path).
	{
		FShore Shore;
		if (!TestTrue(TEXT("Content loaded"), Shore.IsValid()))
		{
			return false;
		}
		Shore.KillScavenger();
		TestFalse(TEXT("No quest yet"), Shore.Quests->HasQuest(Quest));
		TestEqual(TEXT("Mara noticed"), Shore.Talk(), FName(TEXT("already_dead")));
		TestTrue(TEXT("Claim it"), Shore.Say(TEXT("It was me.")));
		TestEqual(TEXT("Quest starts already resolved"), Shore.Stage(), FName(TEXT("return_killed")));
		TestEqual(TEXT("Straight to turn-in"), Shore.Dialogue->GetCurrentNodeId(), FName(TEXT("turnin_kill")));
		TestTrue(TEXT("Turn in"), Shore.Say(TEXT("He's dead. His relay has no one to tend it.")));
		TestEqual(TEXT("Kill outcome"), Shore.Stage(), FName(TEXT("done_killed")));
	}

	// Denying it leaves the quest unstarted and the question open.
	{
		FShore Shore;
		Shore.KillScavenger();
		Shore.Talk();
		Shore.Say(TEXT("Wasn't me."));
		Shore.Say(TEXT("Goodbye."));
		TestFalse(TEXT("Denied: no quest"), Shore.Quests->HasQuest(Quest));
		TestEqual(TEXT("Denied: asked again"), Shore.Talk(), FName(TEXT("already_dead")));
	}

	// Coil taken before meeting Mara.
	{
		FShore Shore;
		Shore.Inventory->AddItem(Shore.Coil, 1);
		TestEqual(TEXT("Greeting"), Shore.Talk(), FName(TEXT("greeting")));
		Shore.Say(TEXT("You keep looking toward his camp."));
		TestTrue(TEXT("Coil shortcut visible"), Shore.Say(TEXT("This coil? I already pulled it.")));
		TestEqual(TEXT("Quest starts at return_coil"), Shore.Stage(), FName(TEXT("return_coil")));
		TestEqual(TEXT("Straight to coil turn-in"), Shore.Dialogue->GetCurrentNodeId(), FName(TEXT("turnin_coil")));
	}

	// Declining leaves no trace.
	{
		FShore Shore;
		Shore.Talk();
		Shore.Say(TEXT("You keep looking toward his camp."));
		TestTrue(TEXT("Decline"), Shore.Say(TEXT("Not my problem.")));
		TestFalse(TEXT("Declined: no quest"), Shore.Quests->HasQuest(Quest));
		TestFalse(TEXT("Declined: dialogue closed"), Shore.Dialogue->IsInDialogue());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCMaraWreckLineTest, "DeadCurrent.Content.Exploration.MaraWreckLine",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDCMaraWreckLineTest::RunTest(const FString& Parameters)
{
	using namespace DCShoreWatchTest;
	LoadContent();
	const FString TellWreck = TEXT("There's a wrecked survey launch west of the boathouse. I read her log.");
	const FName LogRead = TEXT("wreck.log_read");
	const FName Told = TEXT("wreck.mara_told");

	// Before the log is read, Mara never mentions the wreck.
	{
		FShore Shore;
		if (!TestTrue(TEXT("Shipped content loaded"), Shore.IsValid()))
		{
			return false;
		}
		Shore.Talk();
		TestFalse(TEXT("No wreck line before the log"), Shore.Can(TellWreck));

		// After it: one exchange, from her ordinary greeting, without touching Shore Watch.
		Shore.WorldState->SetFlag(LogRead);
		TestEqual(TEXT("Greeting"), Shore.Talk(), FName(TEXT("greeting")));
		TestTrue(TEXT("Tell her about the wreck"), Shore.Say(TellWreck));
		TestEqual(TEXT("Her answer"), Shore.Dialogue->GetCurrentNodeId(), FName(TEXT("wreck")));
		TestTrue(TEXT("Ask"), Shore.Say(TEXT("Was it a storm?")));
		TestEqual(TEXT("Her deflection"), Shore.Dialogue->GetCurrentNodeId(), FName(TEXT("wreck_storm")));
		TestTrue(TEXT("Remembered"), Shore.WorldState->HasFlag(Told));
		TestTrue(TEXT("Shore Watch untouched"), Shore.Quests->GetQuestStatus(Quest) == EDCQuestStatus::NotStarted);
		Shore.Talk();
		TestFalse(TEXT("Asked once"), Shore.Can(TellWreck));
		TestTrue(TEXT("Shore Watch offer still there"), Shore.Can(TEXT("You keep looking toward his camp.")));

		// Save/load keeps it told.
		TUniquePtr<FShore> Reloaded = Shore.SaveAndReload();
		Reloaded->Talk();
		TestFalse(TEXT("Reload: still asked once"), Reloaded->Can(TellWreck));
	}

	// While Shore Watch runs: offered in her in-progress and epilogue talk, never in the turn-in.
	{
		FShore Shore;
		Shore.Talk();
		Shore.Say(TEXT("You keep looking toward his camp."));
		Shore.Say(TEXT("I'll put him down."));
		Shore.WorldState->SetFlag(LogRead);
		TestEqual(TEXT("In progress"), Shore.Talk(), FName(TEXT("inprogress")));
		TestTrue(TEXT("Offered mid-quest"), Shore.Can(TellWreck));

		Shore.KillScavenger();
		TestEqual(TEXT("Turn-in"), Shore.Talk(), FName(TEXT("turnin_kill")));
		TestFalse(TEXT("Not in the turn-in"), Shore.Can(TellWreck));
		TestTrue(TEXT("Turn in"), Shore.Say(TEXT("He's dead. His relay has no one to tend it.")));
		TestEqual(TEXT("Epilogue"), Shore.Talk(), FName(TEXT("done_killed")));
		TestTrue(TEXT("Offered after the quest"), Shore.Say(TellWreck));
		TestEqual(TEXT("Outcome unchanged"), Shore.Stage(), FName(TEXT("done_killed")));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCInspectVariantTest, "DeadCurrent.World.InspectVariants",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCInspectVariantTest::RunTest(const FString& Parameters)
{
	FDCTestWorld World;
	AActor* Player = World.SpawnActor();
	ADCInspectableActor* Rig = World.Get()->SpawnActor<ADCInspectableActor>();
	UDCWorldStateSubsystem* WorldState = World.Get()->GetSubsystem<UDCWorldStateSubsystem>();

	// Relay-rig pattern: the first inspection is a clue that sets a flag; later ones read differently.
	FDCInspectVariant FirstLook;
	FirstLook.Description = FText::FromString(TEXT("Words, almost."));
	FDCGameplayCondition NotSeen;
	NotSeen.Type = EDCConditionType::WorldFlag;
	NotSeen.Id = TEXT("test.seen");
	NotSeen.bNegate = true;
	FirstLook.Conditions = { NotSeen };
	FDCGameplayConsequence MarkSeen;
	MarkSeen.Type = EDCConsequenceType::SetWorldFlag;
	MarkSeen.Id = TEXT("test.seen");
	FirstLook.Consequences = { MarkSeen };

	FArrayProperty* VariantsProperty = CastField<FArrayProperty>(ADCInspectableActor::StaticClass()->FindPropertyByName(TEXT("Variants")));
	if (!TestNotNull(TEXT("Variants property"), VariantsProperty))
	{
		return false;
	}
	*VariantsProperty->ContainerPtrToValuePtr<TArray<FDCInspectVariant>>(Rig) = { FirstLook };

	TestTrue(TEXT("Usable with only variants"), IDCInteractable::Execute_CanInteract(Rig, Player));
	IDCInteractable::Execute_Interact(Rig, Player);
	TestTrue(TEXT("First inspection set the flag"), WorldState->HasFlag(TEXT("test.seen")));
	WorldState->ClearFlag(TEXT("test.seen"));
	WorldState->SetFlag(TEXT("test.other"));
	IDCInteractable::Execute_Interact(Rig, Player);
	TestTrue(TEXT("Variant runs again when its condition passes again"), WorldState->HasFlag(TEXT("test.seen")));

	// Verbs: a variant can rename the prompt ("Pull the leads"), and falls back to the actor's verb.
	TestEqual(TEXT("Default verb"), IDCInteractable::Execute_GetInteractionPrompt(Rig, Player).Action.ToString(), FString(TEXT("Inspect")));
	FDCInspectVariant Pull;
	Pull.Action = FText::FromString(TEXT("Pull the leads"));
	Pull.Conditions = { NotSeen };
	Pull.Consequences = { MarkSeen };
	FDCInspectVariant Done;
	Done.Description = FText::FromString(TEXT("Loose leads."));
	*VariantsProperty->ContainerPtrToValuePtr<TArray<FDCInspectVariant>>(Rig) = { Pull, Done };
	WorldState->ClearFlag(TEXT("test.seen"));
	TestEqual(TEXT("Variant verb"), IDCInteractable::Execute_GetInteractionPrompt(Rig, Player).Action.ToString(), FString(TEXT("Pull the leads")));
	IDCInteractable::Execute_Interact(Rig, Player);
	TestTrue(TEXT("Pulling set the flag"), WorldState->HasFlag(TEXT("test.seen")));
	TestEqual(TEXT("Next variant falls back to the actor verb"), IDCInteractable::Execute_GetInteractionPrompt(Rig, Player).Action.ToString(), FString(TEXT("Inspect")));
	return true;
}

#endif
