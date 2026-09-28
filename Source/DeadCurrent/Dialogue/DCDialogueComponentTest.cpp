#include "Dialogue/DCDialogueAsset.h"
#include "Dialogue/DCDialogueComponent.h"
#include "Quest/DCQuestComponent.h"
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCDialogueConditionTest, "DeadCurrent.Dialogue.Conditions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCDialogueConditionTest::RunTest(const FString& Parameters)
{
	const FName QuestId = TEXT("test.dialogue");

	UDCDialogueAsset* Asset = NewObject<UDCDialogueAsset>();
	Asset->DialogueId = TEXT("test_gated");
	Asset->EntryNodeId = TEXT("greeting");

	FDCDialogueEntry DoneEntry;
	DoneEntry.NodeId = TEXT("done");
	FDCGameplayCondition Complete;
	Complete.Type = EDCConditionType::QuestComplete;
	Complete.Id = QuestId;
	DoneEntry.Conditions = { Complete };

	FDCDialogueEntry GreetingEntry;
	GreetingEntry.NodeId = TEXT("greeting");

	Asset->Entries = { DoneEntry, GreetingEntry };

	FDCDialogueNode Greeting;
	Greeting.NodeId = TEXT("greeting");
	Greeting.Speaker = FText::FromString(TEXT("Mara"));
	Greeting.Line = FText::FromString(TEXT("Work to do."));
	FDCDialogueChoice Take;
	Take.Text = FText::FromString(TEXT("I'll do it."));
	Take.NextNodeId = TEXT("accept");
	FDCGameplayConsequence Start;
	Start.Type = EDCConsequenceType::StartQuest;
	Start.Id = QuestId;
	Start.Stage = TEXT("accepted");
	Take.Consequences = { Start };
	FDCDialogueChoice Bye;
	Bye.Text = FText::FromString(TEXT("Goodbye"));
	Greeting.Choices = { Take, Bye };

	FDCDialogueNode Accept;
	Accept.NodeId = TEXT("accept");
	Accept.Speaker = FText::FromString(TEXT("Mara"));
	Accept.Line = FText::FromString(TEXT("Good."));
	FDCDialogueChoice AcceptBye;
	AcceptBye.Text = FText::FromString(TEXT("Goodbye"));
	Accept.Choices = { AcceptBye };

	FDCDialogueNode Done;
	Done.NodeId = TEXT("done");
	Done.Speaker = FText::FromString(TEXT("Mara"));
	Done.Line = FText::FromString(TEXT("It's done."));
	FDCDialogueChoice DoneBye;
	DoneBye.Text = FText::FromString(TEXT("Goodbye"));
	Done.Choices = { DoneBye };

	Asset->Nodes = { Greeting, Accept, Done };

	UDCQuestComponent* Quests = NewObject<UDCQuestComponent>();
	TestEqual(TEXT("Default entry"), Asset->ResolveEntry(Quests), FName(TEXT("greeting")));

	TestTrue(TEXT("Complete then pick done entry"), Quests->StartQuest(QuestId, TEXT("done")));
	TestTrue(TEXT("Marked complete"), Quests->IsComplete(QuestId));
	TestEqual(TEXT("Done entry"), Asset->ResolveEntry(Quests), FName(TEXT("done")));

	UDCQuestComponent* Fresh = NewObject<UDCQuestComponent>();
	TestEqual(TEXT("No quest uses greeting"), Asset->ResolveEntry(Fresh), FName(TEXT("greeting")));
	TestEqual(TEXT("Null quests uses first empty-condition entry"), Asset->ResolveEntry(nullptr), FName(TEXT("greeting")));

	return true;
}

#endif
