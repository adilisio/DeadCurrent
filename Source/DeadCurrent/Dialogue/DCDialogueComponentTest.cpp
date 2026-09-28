#include "Dialogue/DCDialogueAsset.h"
#include "Dialogue/DCDialogueComponent.h"
#include "Core/DCGameplayRules.h"
#include "Core/DCTestHelpers.h"
#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Quest/DCQuestComponent.h"
#include "Quest/DCQuestDefinition.h"
#include "World/DCWorldStateSubsystem.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCDialogueBranchingTest, "DeadCurrent.Dialogue.Branching",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCDialogueBranchingTest::RunTest(const FString& Parameters)
{
	UDCDialogueAsset* Asset = NewObject<UDCDialogueAsset>();
	Asset->DialogueId = TEXT("test_greeting");
	Asset->EntryNodeId = TEXT("greeting");

	FDCDialogueNode Greeting;
	Greeting.NodeId = TEXT("greeting");
	Greeting.Speaker = FText::FromString(TEXT("Mara"));
	Greeting.Line = FText::FromString(TEXT("Hello."));
	Greeting.Choices = {
		{ FText::FromString(TEXT("Who are you?")), TEXT("who") },
		{ FText::FromString(TEXT("What is this place?")), TEXT("place") },
		{ FText::FromString(TEXT("Goodbye")), NAME_None },
	};

	FDCDialogueNode Who;
	Who.NodeId = TEXT("who");
	Who.Speaker = FText::FromString(TEXT("Mara"));
	Who.Line = FText::FromString(TEXT("Mara."));
	Who.Choices = {
		{ FText::FromString(TEXT("What is this place?")), TEXT("place") },
		{ FText::FromString(TEXT("Goodbye")), NAME_None },
	};

	FDCDialogueNode Place;
	Place.NodeId = TEXT("place");
	Place.Speaker = FText::FromString(TEXT("Mara"));
	Place.Line = FText::FromString(TEXT("Shore."));
	Place.Choices = {
		{ FText::FromString(TEXT("Who are you?")), TEXT("who") },
		{ FText::FromString(TEXT("Goodbye")), NAME_None },
	};

	Asset->Nodes = { Greeting, Who, Place };

	UDCDialogueComponent* Dialogue = NewObject<UDCDialogueComponent>();
	TestFalse(TEXT("Null asset is rejected"), Dialogue->StartDialogue(nullptr, nullptr));
	TestTrue(TEXT("Start opens entry"), Dialogue->StartDialogue(Asset, nullptr));
	TestTrue(TEXT("Is talking"), Dialogue->IsInDialogue());
	TestEqual(TEXT("Entry node"), Dialogue->GetCurrentNodeId(), FName(TEXT("greeting")));
	TestEqual(TEXT("Three choices"), Dialogue->GetCurrentNode()->Choices.Num(), 3);

	TestFalse(TEXT("Out of range choice"), Dialogue->SelectChoice(9));
	TestTrue(TEXT("Who branch"), Dialogue->SelectChoice(0));
	TestEqual(TEXT("Who node"), Dialogue->GetCurrentNodeId(), FName(TEXT("who")));

	TestTrue(TEXT("Place branch"), Dialogue->SelectChoice(0));
	TestEqual(TEXT("Place node"), Dialogue->GetCurrentNodeId(), FName(TEXT("place")));

	TestTrue(TEXT("Goodbye ends"), Dialogue->SelectChoice(1));
	TestFalse(TEXT("Not talking after goodbye"), Dialogue->IsInDialogue());

	TestTrue(TEXT("Restart works"), Dialogue->StartDialogue(Asset, nullptr));
	TestTrue(TEXT("Immediate goodbye"), Dialogue->SelectChoice(2));
	TestFalse(TEXT("Ended"), Dialogue->IsInDialogue());

	return true;
}

namespace DCDialogueTest
{
	FDCGameplayCondition Cond(EDCConditionType Type, FName Id, FName Stage = NAME_None, bool bNegate = false)
	{
		FDCGameplayCondition C;
		C.Type = Type;
		C.Id = Id;
		C.Stage = Stage;
		C.bNegate = bNegate;
		return C;
	}

	FDCGameplayConsequence Cons(EDCConsequenceType Type, FName Id, FName Stage = NAME_None, int32 Quantity = 1)
	{
		FDCGameplayConsequence C;
		C.Type = Type;
		C.Id = Id;
		C.Stage = Stage;
		C.Quantity = Quantity;
		return C;
	}

	FDCDialogueChoice Choice(const TCHAR* Text, FName Next, TArray<FDCGameplayCondition> Conditions = {}, TArray<FDCGameplayConsequence> Consequences = {})
	{
		FDCDialogueChoice C;
		C.Text = FText::FromString(Text);
		C.NextNodeId = Next;
		C.Conditions = MoveTemp(Conditions);
		C.Consequences = MoveTemp(Consequences);
		return C;
	}

	FDCDialogueNode Node(FName Id, const TCHAR* Line, TArray<FDCDialogueChoice> Choices)
	{
		FDCDialogueNode N;
		N.NodeId = Id;
		N.Speaker = FText::FromString(TEXT("Mara"));
		N.Line = FText::FromString(Line);
		N.Choices = MoveTemp(Choices);
		return N;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCDialogueConditionTest, "DeadCurrent.Dialogue.Conditions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCDialogueConditionTest::RunTest(const FString& Parameters)
{
	using namespace DCDialogueTest;

	const FName QuestId = TEXT("test.dialogue");
	UDCQuestDefinition* Quest = NewObject<UDCQuestDefinition>();
	Quest->QuestId = QuestId;
	FDCQuestStage Accepted;
	Accepted.StageId = TEXT("accepted");
	FDCQuestStage Done;
	Done.StageId = TEXT("done");
	Done.bCompletesQuest = true;
	Quest->Stages = { Accepted, Done };

	UDCDialogueAsset* Asset = NewObject<UDCDialogueAsset>();
	Asset->DialogueId = TEXT("test_gated");
	Asset->EntryNodeId = TEXT("greeting");
	Asset->Entries = {
		{ TEXT("done"), { Cond(EDCConditionType::QuestComplete, QuestId) } },
		{ TEXT("missing_node"), {} },
		{ TEXT("busy"), { Cond(EDCConditionType::QuestActive, QuestId) } },
	};
	Asset->Nodes = {
		Node(TEXT("greeting"), TEXT("Work to do."), { Choice(TEXT("Goodbye"), NAME_None) }),
		Node(TEXT("busy"), TEXT("Still waiting."), { Choice(TEXT("Goodbye"), NAME_None) }),
		Node(TEXT("done"), TEXT("It's done."), { Choice(TEXT("Goodbye"), NAME_None) }),
	};

	FDCTestWorld World;
	AActor* Player = World.SpawnActor();
	UDCQuestComponent* Quests = FDCTestWorld::AddComponent<UDCQuestComponent>(Player);

	TestEqual(TEXT("Default entry"), Asset->ResolveEntry(FDCRuleContext::ForActor(Player)), FName(TEXT("greeting")));
	Quests->StartQuest(QuestId);
	TestEqual(TEXT("Active entry (missing node entry skipped)"), Asset->ResolveEntry(FDCRuleContext::ForActor(Player)), FName(TEXT("busy")));
	Quests->SetStage(QuestId, TEXT("done"));
	TestEqual(TEXT("Done entry"), Asset->ResolveEntry(FDCRuleContext::ForActor(Player)), FName(TEXT("done")));
	TestEqual(TEXT("Empty context falls back"), Asset->ResolveEntry(FDCRuleContext()), FName(TEXT("greeting")));

	UDCDialogueComponent* Dialogue = FDCTestWorld::AddComponent<UDCDialogueComponent>(Player);
	TestTrue(TEXT("Start uses resolved entry"), Dialogue->StartDialogue(Asset, nullptr));
	TestEqual(TEXT("Opened at done"), Dialogue->GetCurrentNodeId(), FName(TEXT("done")));

	Quest->QuestId = NAME_None;
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCDialogueConsequenceTest, "DeadCurrent.Dialogue.Consequences",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCDialogueConsequenceTest::RunTest(const FString& Parameters)
{
	using namespace DCDialogueTest;

	const FName QuestId = TEXT("test.dialogue_quest");
	UDCQuestDefinition* Quest = NewObject<UDCQuestDefinition>();
	Quest->QuestId = QuestId;
	FDCQuestStage Accepted;
	Accepted.StageId = TEXT("accepted");
	FDCQuestStage Done;
	Done.StageId = TEXT("done");
	Done.bCompletesQuest = true;
	Quest->Stages = { Accepted, Done };

	UDCItemDefinition* Key = NewObject<UDCItemDefinition>();
	Key->ItemId = TEXT("test.dialogue_key");
	UDCItemDefinition* Reward = NewObject<UDCItemDefinition>();
	Reward->ItemId = TEXT("test.dialogue_reward");
	Reward->MaxStackSize = 99;

	UDCDialogueAsset* Asset = NewObject<UDCDialogueAsset>();
	Asset->EntryNodeId = TEXT("offer");
	Asset->Nodes = {
		Node(TEXT("offer"), TEXT("Help me?"), {
			Choice(TEXT("Yes."), TEXT("thanks"), { Cond(EDCConditionType::QuestNotStarted, QuestId) },
				{ Cons(EDCConsequenceType::StartQuest, QuestId) }),
			Choice(TEXT("I have the key."), TEXT("thanks"), { Cond(EDCConditionType::HasItem, Key->ItemId) }, {
				Cons(EDCConsequenceType::RemoveItem, Key->ItemId),
				Cons(EDCConsequenceType::GiveItem, Reward->ItemId, NAME_None, 24),
				Cons(EDCConsequenceType::SetQuestStage, QuestId, TEXT("done")),
				Cons(EDCConsequenceType::SetWorldFlag, TEXT("test.dialogue_done")) }),
			Choice(TEXT("Goodbye"), NAME_None),
		}),
		Node(TEXT("thanks"), TEXT("Thanks."), { Choice(TEXT("Goodbye"), NAME_None) }),
	};

	FDCTestWorld World;
	AActor* Player = World.SpawnActor();
	UDCInventoryComponent* Inventory = FDCTestWorld::AddComponent<UDCInventoryComponent>(Player);
	UDCQuestComponent* Quests = FDCTestWorld::AddComponent<UDCQuestComponent>(Player);
	UDCDialogueComponent* Dialogue = FDCTestWorld::AddComponent<UDCDialogueComponent>(Player);
	UDCWorldStateSubsystem* WorldState = World.Get()->GetSubsystem<UDCWorldStateSubsystem>();

	// Without the key, only "Yes." and "Goodbye" are visible; digit 1 is "Yes.".
	TestTrue(TEXT("Start"), Dialogue->StartDialogue(Asset, nullptr));
	TestEqual(TEXT("Two visible choices"), Dialogue->GetVisibleChoiceIndices(), TArray<int32>({ 0, 2 }));
	TestTrue(TEXT("Accept"), Dialogue->SelectChoice(0));
	TestTrue(TEXT("Choice started quest"), Quests->IsQuestActive(QuestId));
	TestEqual(TEXT("Advanced to thanks"), Dialogue->GetCurrentNodeId(), FName(TEXT("thanks")));
	Dialogue->SelectChoice(0);
	TestFalse(TEXT("Ended"), Dialogue->IsInDialogue());

	// Quest started, no key: only Goodbye.
	Dialogue->StartDialogue(Asset, nullptr);
	TestEqual(TEXT("Only goodbye"), Dialogue->GetVisibleChoiceIndices(), TArray<int32>({ 2 }));
	TestFalse(TEXT("Hidden choice can't be picked"), Dialogue->SelectChoice(1));
	Dialogue->EndDialogue();

	// With the key the turn-in appears and its consequences run in order.
	Inventory->AddItem(Key, 1);
	Dialogue->StartDialogue(Asset, nullptr);
	TestEqual(TEXT("Turn-in visible"), Dialogue->GetVisibleChoiceIndices(), TArray<int32>({ 1, 2 }));
	TestTrue(TEXT("Turn in"), Dialogue->SelectChoice(0));
	TestEqual(TEXT("Key taken"), Inventory->GetQuantity(Key), 0);
	TestEqual(TEXT("Reward given"), Inventory->GetQuantity(Reward), 24);
	TestTrue(TEXT("Quest complete"), Quests->IsComplete(QuestId));
	TestTrue(TEXT("Flag set"), WorldState->HasFlag(TEXT("test.dialogue_done")));

	Quest->QuestId = NAME_None;
	Key->ItemId = NAME_None;
	Reward->ItemId = NAME_None;
	return true;
}

#endif
