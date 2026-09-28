#include "Core/DCGameplayTypes.h"
#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Quest/DCQuestComponent.h"
#include "Quest/DCQuestDefinition.h"
#include "Save/DCPersistentTypes.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCQuestStageTest, "DeadCurrent.Quest.Stages",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCQuestStageTest::RunTest(const FString& Parameters)
{
	UDCQuestComponent* Quests = NewObject<UDCQuestComponent>();
	const FName QuestId = TEXT("test.watch");

	auto Cond = [](EDCConditionType Type, FName Id, FName Stage = NAME_None)
	{
		FDCGameplayCondition C;
		C.Type = Type;
		C.Id = Id;
		C.Stage = Stage;
		return C;
	};

	TestFalse(TEXT("Not started"), Quests->HasQuest(QuestId));
	TestTrue(TEXT("NotStarted condition"), Quests->Meets(Cond(EDCConditionType::QuestNotStarted, QuestId)));
	TestFalse(TEXT("Null start rejected"), Quests->StartQuest(NAME_None, TEXT("accepted")));
	TestTrue(TEXT("Start"), Quests->StartQuest(QuestId, TEXT("accepted")));
	TestFalse(TEXT("Cannot start twice"), Quests->StartQuest(QuestId, TEXT("accepted")));
	TestEqual(TEXT("Stage is accepted"), Quests->GetStage(QuestId), FName(TEXT("accepted")));
	TestTrue(TEXT("Active"), Quests->IsQuestActive(QuestId));
	TestTrue(TEXT("QuestStage condition"), Quests->Meets(Cond(EDCConditionType::QuestStage, QuestId, TEXT("accepted"))));
	TestTrue(TEXT("QuestActive condition"), Quests->Meets(Cond(EDCConditionType::QuestActive, QuestId)));
	TestFalse(TEXT("Not complete"), Quests->IsComplete(QuestId));
	TestFalse(TEXT("HostileDead without world"), Quests->Meets(Cond(EDCConditionType::HostileDead, NAME_None)));

	TestTrue(TEXT("Advance"), Quests->SetStage(QuestId, TEXT("return")));
	TestEqual(TEXT("Stage is return"), Quests->GetStage(QuestId), FName(TEXT("return")));

	TestTrue(TEXT("Complete"), Quests->CompleteQuest(QuestId));
	TestTrue(TEXT("Is complete"), Quests->IsComplete(QuestId));
	TestFalse(TEXT("No longer active"), Quests->IsQuestActive(QuestId));
	TestTrue(TEXT("QuestComplete condition"), Quests->Meets(Cond(EDCConditionType::QuestComplete, QuestId)));

	Quests->SetFlag(TEXT("shore.cleared"));
	TestTrue(TEXT("Flag set"), Quests->HasFlag(TEXT("shore.cleared")));
	TestTrue(TEXT("WorldFlag condition"), Quests->Meets(Cond(EDCConditionType::WorldFlag, TEXT("shore.cleared"))));
	TestFalse(TEXT("Missing flag"), Quests->HasFlag(TEXT("missing")));

	FDCGameplayCondition Negated;
	Negated.Type = EDCConditionType::WorldFlag;
	Negated.Id = TEXT("shore.cleared");
	Negated.bNegate = true;
	TestFalse(TEXT("Negated flag fails when set"), Quests->Meets(Negated));

	TArray<FDCSavedQuestState> SavedQuests;
	TArray<FName> SavedFlags;
	Quests->CaptureState(SavedQuests, SavedFlags);
	TestEqual(TEXT("Captured one quest"), SavedQuests.Num(), 1);
	TestEqual(TEXT("Captured done stage"), SavedQuests[0].StageId, FName(TEXT("done")));
	TestEqual(TEXT("Captured flag"), SavedFlags.Num(), 1);

	UDCQuestComponent* Restored = NewObject<UDCQuestComponent>();
	Restored->ReplaceFromSaved(SavedQuests, SavedFlags);
	TestTrue(TEXT("Restore complete"), Restored->IsComplete(QuestId));
	TestTrue(TEXT("Restore flag"), Restored->HasFlag(TEXT("shore.cleared")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCQuestConsequenceTest, "DeadCurrent.Quest.Consequences",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCQuestConsequenceTest::RunTest(const FString& Parameters)
{
	UDCQuestComponent* Quests = NewObject<UDCQuestComponent>();

	FDCGameplayConsequence Start;
	Start.Type = EDCConsequenceType::StartQuest;
	Start.Id = TEXT("test.watch");
	Start.Stage = TEXT("accepted");
	Quests->Apply(Start);
	TestTrue(TEXT("StartQuest consequence"), Quests->IsQuestActive(TEXT("test.watch")));

	FDCGameplayConsequence Advance;
	Advance.Type = EDCConsequenceType::SetQuestStage;
	Advance.Id = TEXT("test.watch");
	Advance.Stage = TEXT("return");
	Quests->Apply(Advance);
	TestEqual(TEXT("SetQuestStage consequence"), Quests->GetStage(TEXT("test.watch")), FName(TEXT("return")));

	FDCGameplayConsequence Flag;
	Flag.Type = EDCConsequenceType::SetWorldFlag;
	Flag.Id = TEXT("shore.cleared");
	Quests->Apply(Flag);

	FDCGameplayConsequence Done;
	Done.Type = EDCConsequenceType::CompleteQuest;
	Done.Id = TEXT("test.watch");
	Quests->Apply(Done);
	TestTrue(TEXT("CompleteQuest consequence"), Quests->IsComplete(TEXT("test.watch")));
	TestTrue(TEXT("Flag from consequence"), Quests->HasFlag(TEXT("shore.cleared")));

	FDCGameplayCondition ActiveAndFlag;
	ActiveAndFlag.Type = EDCConditionType::QuestActive;
	ActiveAndFlag.Id = TEXT("test.watch");
	TestFalse(TEXT("No longer active after complete"), Quests->Meets(ActiveAndFlag));
	TestTrue(TEXT("MeetsAll empty"), Quests->MeetsAll({}));

	UDCQuestDefinition* Def = NewObject<UDCQuestDefinition>();
	Def->QuestId = TEXT("test.death");
	Def->CompletedStage = TEXT("done");
	FDCQuestStage Accepted;
	Accepted.StageId = TEXT("accepted");
	Accepted.bAdvanceOnHostileDeath = true;
	Accepted.NextStageOnHostileDeath = TEXT("return");
	Def->Stages = { Accepted };

	UDCQuestComponent* DeathQuest = NewObject<UDCQuestComponent>();
	TestTrue(TEXT("Start death quest"), DeathQuest->StartQuest(TEXT("test.death"), TEXT("accepted")));
	DeathQuest->NotifyHostileDied();
	TestEqual(TEXT("Hostile death advances stage"), DeathQuest->GetStage(TEXT("test.death")), FName(TEXT("return")));

	return true;
}

#endif
