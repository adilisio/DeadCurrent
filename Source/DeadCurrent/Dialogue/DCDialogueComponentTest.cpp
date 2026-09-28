#include "Dialogue/DCDialogueAsset.h"
#include "Dialogue/DCDialogueComponent.h"
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

#endif
