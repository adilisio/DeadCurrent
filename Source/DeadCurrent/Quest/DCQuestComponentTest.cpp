#include "Core/DCGameplayRules.h"
#include "Core/DCTestHelpers.h"
#include "Combat/DCHealthComponent.h"
#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Kismet/GameplayStatics.h"
#include "Quest/DCQuestComponent.h"
#include "Quest/DCQuestDefinition.h"
#include "Save/DCPersistentIdComponent.h"
#include "Save/DCSaveGame.h"
#include "Save/DCSaveSubsystem.h"
#include "World/DCWorldStateSubsystem.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace DCQuestTest
{
	const FName QuestId = TEXT("test.branching");
	const FName TargetId = TEXT("test.quest_target");

	FDCGameplayCondition Cond(EDCConditionType Type, FName Id)
	{
		FDCGameplayCondition C;
		C.Type = Type;
		C.Id = Id;
		return C;
	}

	FDCGameplayConsequence Cons(EDCConsequenceType Type, FName Id, int32 Quantity = 1)
	{
		FDCGameplayConsequence C;
		C.Type = Type;
		C.Id = Id;
		C.Quantity = Quantity;
		return C;
	}

	/**
	 *  accepted --(target dead)--> return_force --(dialogue)--> done_force  [flag test.force]
	 *           --(has token)----> return_token --(dialogue)--> done_token  [flag test.token, +1 reward]
	 */
	UDCQuestDefinition* MakeBranchingQuest(FName TokenId, FName RewardId)
	{
		UDCQuestDefinition* Quest = NewObject<UDCQuestDefinition>();
		Quest->QuestId = QuestId;
		Quest->DisplayName = FText::FromString(TEXT("Branching"));

		FDCQuestStage Accepted;
		Accepted.StageId = TEXT("accepted");
		Accepted.ObjectiveText = FText::FromString(TEXT("Deal with it."));
		FDCQuestTransition ByForce;
		ByForce.Conditions = { Cond(EDCConditionType::ActorDead, TargetId) };
		ByForce.NextStage = TEXT("return_force");
		FDCQuestTransition ByToken;
		ByToken.Conditions = { Cond(EDCConditionType::HasItem, TokenId) };
		ByToken.NextStage = TEXT("return_token");
		Accepted.Transitions = { ByForce, ByToken };

		FDCQuestStage ReturnForce;
		ReturnForce.StageId = TEXT("return_force");
		ReturnForce.ObjectiveText = FText::FromString(TEXT("Report the kill."));

		FDCQuestStage ReturnToken;
		ReturnToken.StageId = TEXT("return_token");
		ReturnToken.ObjectiveText = FText::FromString(TEXT("Hand over the token."));

		FDCQuestStage DoneForce;
		DoneForce.StageId = TEXT("done_force");
		DoneForce.bCompletesQuest = true;
		DoneForce.ObjectiveText = FText::FromString(TEXT("Done by force."));
		DoneForce.OnEnter = { Cons(EDCConsequenceType::SetWorldFlag, TEXT("test.force")) };

		FDCQuestStage DoneToken;
		DoneToken.StageId = TEXT("done_token");
		DoneToken.bCompletesQuest = true;
		DoneToken.ObjectiveText = FText::FromString(TEXT("Done quietly."));
		DoneToken.OnEnter = {
			Cons(EDCConsequenceType::SetWorldFlag, TEXT("test.token")),
			Cons(EDCConsequenceType::GiveItem, RewardId, 1) };

		Quest->Stages = { Accepted, ReturnForce, ReturnToken, DoneForce, DoneToken };
		return Quest;
	}

	UDCItemDefinition* MakeItem(FName ItemId)
	{
		UDCItemDefinition* Item = NewObject<UDCItemDefinition>();
		Item->ItemId = ItemId;
		Item->MaxStackSize = 10;
		return Item;
	}

	struct FQuestScene
	{
		AActor* Player = nullptr;
		UDCInventoryComponent* Inventory = nullptr;
		UDCQuestComponent* Quests = nullptr;
		UDCHealthComponent* TargetHealth = nullptr;
		UDCWorldStateSubsystem* WorldState = nullptr;

		explicit FQuestScene(const FDCTestWorld& World)
		{
			Player = World.SpawnActor();
			Inventory = FDCTestWorld::AddComponent<UDCInventoryComponent>(Player);
			Quests = FDCTestWorld::AddComponent<UDCQuestComponent>(Player);

			AActor* Target = World.SpawnActor();
			FDCTestWorld::AddComponent<UDCPersistentIdComponent>(Target, [](UDCPersistentIdComponent* C) { C->SetPersistentId(TargetId); });
			TargetHealth = FDCTestWorld::AddComponent<UDCHealthComponent>(Target);
			WorldState = World.Get()->GetSubsystem<UDCWorldStateSubsystem>();
		}

		void KillTarget() const
		{
			FDCDamageInfo Damage;
			Damage.Amount = TargetHealth->GetMaxHealth();
			TargetHealth->ApplyDamage(Damage);
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCQuestStageTest, "DeadCurrent.Quest.Stages",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCQuestStageTest::RunTest(const FString& Parameters)
{
	using namespace DCQuestTest;

	UDCItemDefinition* Token = MakeItem(TEXT("test.stage_token"));
	UDCItemDefinition* Reward = MakeItem(TEXT("test.stage_reward"));
	UDCQuestDefinition* Quest = MakeBranchingQuest(Token->ItemId, Reward->ItemId);
	UDCQuestComponent* Quests = NewObject<UDCQuestComponent>();

	TestEqual(TEXT("Start stage defaults to first"), Quest->GetStartStage(), FName(TEXT("accepted")));
	TestEqual(TEXT("Not started"), Quests->GetQuestStatus(QuestId), EDCQuestStatus::NotStarted);
	TestFalse(TEXT("Null id rejected"), Quests->StartQuest(NAME_None));
	AddExpectedError(TEXT("cannot start"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("Unknown start stage rejected"), Quests->StartQuest(QuestId, TEXT("nope")));
	TestFalse(TEXT("SetStage before start rejected"), Quests->SetStage(QuestId, TEXT("return_force")));

	TestTrue(TEXT("Start"), Quests->StartQuest(QuestId));
	TestFalse(TEXT("Cannot start twice"), Quests->StartQuest(QuestId));
	TestEqual(TEXT("At start stage"), Quests->GetStage(QuestId), FName(TEXT("accepted")));
	TestEqual(TEXT("Active"), Quests->GetQuestStatus(QuestId), EDCQuestStatus::Active);
	TestEqual(TEXT("Objective text"), Quests->GetObjectiveText().ToString(), FString(TEXT("Deal with it.")));

	AddExpectedError(TEXT("has no stage"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("Unknown stage rejected"), Quests->SetStage(QuestId, TEXT("nope")));
	TestEqual(TEXT("Stage unchanged"), Quests->GetStage(QuestId), FName(TEXT("accepted")));

	TestTrue(TEXT("Move to return"), Quests->SetStage(QuestId, TEXT("return_force")));
	TestEqual(TEXT("Return objective"), Quests->GetObjectiveText().ToString(), FString(TEXT("Report the kill.")));
	TestTrue(TEXT("Move to outcome"), Quests->SetStage(QuestId, TEXT("done_force")));
	TestEqual(TEXT("Complete"), Quests->GetQuestStatus(QuestId), EDCQuestStatus::Complete);
	TestTrue(TEXT("No objective once complete"), Quests->GetObjectiveText().IsEmpty());
	TestEqual(TEXT("Outcome summary"), Quests->GetStageText(QuestId).ToString(), FString(TEXT("Done by force.")));
	TestEqual(TEXT("Log keeps the quest"), Quests->GetQuestLog().Num(), 1);

	Quest->QuestId = NAME_None;
	Token->ItemId = NAME_None;
	Reward->ItemId = NAME_None;
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCQuestBranchTest, "DeadCurrent.Quest.Branches",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCQuestBranchTest::RunTest(const FString& Parameters)
{
	using namespace DCQuestTest;

	UDCItemDefinition* Token = MakeItem(TEXT("test.branch_token"));
	UDCItemDefinition* Reward = MakeItem(TEXT("test.branch_reward"));
	UDCQuestDefinition* Quest = MakeBranchingQuest(Token->ItemId, Reward->ItemId);

	// Route A: a death in the world advances the quest with no quest-specific code.
	{
		FDCTestWorld World;
		FQuestScene Scene(World);
		TestTrue(TEXT("A: start"), Scene.Quests->StartQuest(QuestId));
		Scene.KillTarget();
		TestEqual(TEXT("A: death moves to return_force"), Scene.Quests->GetStage(QuestId), FName(TEXT("return_force")));

		// Picking the token up afterwards does not switch routes: return stages have no transitions.
		Scene.Inventory->AddItem(Token, 1);
		TestEqual(TEXT("A: route kept"), Scene.Quests->GetStage(QuestId), FName(TEXT("return_force")));

		Scene.Quests->SetStage(QuestId, TEXT("done_force"));
		TestTrue(TEXT("A: complete"), Scene.Quests->IsComplete(QuestId));
		TestTrue(TEXT("A: outcome flag"), Scene.WorldState->HasFlag(TEXT("test.force")));
		TestFalse(TEXT("A: no token flag"), Scene.WorldState->HasFlag(TEXT("test.token")));
		TestEqual(TEXT("A: no token reward"), Scene.Inventory->GetQuantity(Reward), 0);
	}

	// Route B: picking up an item advances the quest; the outcome stage's OnEnter pays out.
	{
		FDCTestWorld World;
		FQuestScene Scene(World);
		TestTrue(TEXT("B: start"), Scene.Quests->StartQuest(QuestId));
		Scene.Inventory->AddItem(Token, 1);
		TestEqual(TEXT("B: item moves to return_token"), Scene.Quests->GetStage(QuestId), FName(TEXT("return_token")));

		Scene.KillTarget();
		TestEqual(TEXT("B: route kept after a later kill"), Scene.Quests->GetStage(QuestId), FName(TEXT("return_token")));

		Scene.Quests->SetStage(QuestId, TEXT("done_token"));
		TestTrue(TEXT("B: complete"), Scene.Quests->IsComplete(QuestId));
		TestTrue(TEXT("B: outcome flag"), Scene.WorldState->HasFlag(TEXT("test.token")));
		TestFalse(TEXT("B: no force flag"), Scene.WorldState->HasFlag(TEXT("test.force")));
		TestEqual(TEXT("B: OnEnter reward"), Scene.Inventory->GetQuantity(Reward), 1);
	}

	// Already satisfied when accepted: starting the quest jumps straight to the return stage.
	{
		FDCTestWorld World;
		FQuestScene Scene(World);
		Scene.Inventory->AddItem(Token, 1);
		TestTrue(TEXT("Pre: start"), Scene.Quests->StartQuest(QuestId));
		TestEqual(TEXT("Pre: jumps to return_token"), Scene.Quests->GetStage(QuestId), FName(TEXT("return_token")));
	}

	// When both are already true, transition order decides (force is listed first).
	{
		FDCTestWorld World;
		FQuestScene Scene(World);
		Scene.Inventory->AddItem(Token, 1);
		Scene.KillTarget();
		TestTrue(TEXT("Both: start"), Scene.Quests->StartQuest(QuestId));
		TestEqual(TEXT("Both: first transition wins"), Scene.Quests->GetStage(QuestId), FName(TEXT("return_force")));
	}

	Quest->QuestId = NAME_None;
	Token->ItemId = NAME_None;
	Reward->ItemId = NAME_None;
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCQuestPersistenceTest, "DeadCurrent.Quest.Persistence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCQuestPersistenceTest::RunTest(const FString& Parameters)
{
	using namespace DCQuestTest;

	const FString Slot = TEXT("DeadCurrent_TestQuest");
	UDCItemDefinition* Token = MakeItem(TEXT("test.persist_token"));
	UDCItemDefinition* Reward = MakeItem(TEXT("test.persist_reward"));
	UDCQuestDefinition* Quest = MakeBranchingQuest(Token->ItemId, Reward->ItemId);

	// Finish route B, then save quest log and world flags through a real slot.
	{
		FDCTestWorld World;
		FQuestScene Scene(World);
		Scene.Quests->StartQuest(QuestId);
		Scene.Inventory->AddItem(Token, 1);
		Scene.Quests->SetStage(QuestId, TEXT("done_token"));
		Scene.WorldState->SetFlag(TEXT("test.extra"));

		UDCSaveGame* Save = NewObject<UDCSaveGame>();
		UDCSaveSubsystem::CaptureProgress(Save, Scene.Quests, Scene.WorldState);
		Save->Quests.Add({ TEXT("test.stale"), TEXT("gone") });
		TestTrue(TEXT("Wrote slot"), UGameplayStatics::SaveGameToSlot(Save, Slot, 0));
	}

	UDCSaveGame* Loaded = Cast<UDCSaveGame>(UGameplayStatics::LoadGameFromSlot(Slot, 0));
	TestNotNull(TEXT("Loaded slot"), Loaded);
	if (!Loaded)
	{
		return false;
	}
	TestEqual(TEXT("Quests round-trip"), Loaded->Quests.Num(), 2);
	TestEqual(TEXT("Flags round-trip"), Loaded->WorldFlags.Num(), 2);

	// Restore into a fresh world: exact stage and flags, and OnEnter does not pay out again.
	{
		FDCTestWorld World;
		FQuestScene Scene(World);
		UDCSaveSubsystem::ApplyProgress(Loaded, Scene.Quests, Scene.WorldState);
		TestEqual(TEXT("Stage restored"), Scene.Quests->GetStage(QuestId), FName(TEXT("done_token")));
		TestTrue(TEXT("Complete after restore"), Scene.Quests->IsComplete(QuestId));
		TestTrue(TEXT("Outcome flag restored"), Scene.WorldState->HasFlag(TEXT("test.token")));
		TestTrue(TEXT("Other flag restored"), Scene.WorldState->HasFlag(TEXT("test.extra")));
		TestEqual(TEXT("OnEnter not re-applied"), Scene.Inventory->GetQuantity(Reward), 0);
		TestTrue(TEXT("Quest with no definition kept"), Scene.Quests->HasQuest(TEXT("test.stale")));
	}

	// A mid-quest save restores the stage and the quest keeps listening afterwards.
	{
		UDCSaveGame* Mid = NewObject<UDCSaveGame>();
		Mid->Quests.Add({ QuestId, TEXT("accepted") });
		Mid->Quests.Add({ QuestId, TEXT("return_force") });

		FDCTestWorld World;
		FQuestScene Scene(World);
		UDCSaveSubsystem::ApplyProgress(Mid, Scene.Quests, Scene.WorldState);
		TestEqual(TEXT("Duplicate entries ignored"), Scene.Quests->GetQuestLog().Num(), 1);
		TestEqual(TEXT("Mid stage restored"), Scene.Quests->GetStage(QuestId), FName(TEXT("accepted")));
		Scene.Inventory->AddItem(Token, 1);
		TestEqual(TEXT("Transitions live after restore"), Scene.Quests->GetStage(QuestId), FName(TEXT("return_token")));
	}

	// A stage removed from the quest data is dropped so the quest can be picked up again.
	{
		UDCSaveGame* Old = NewObject<UDCSaveGame>();
		Old->Quests.Add({ QuestId, TEXT("removed_stage") });
		UDCQuestComponent* Quests = NewObject<UDCQuestComponent>();
		AddExpectedError(TEXT("no longer exists"), EAutomationExpectedErrorFlags::Contains, 1);
		UDCSaveSubsystem::ApplyProgress(Old, Quests, nullptr);
		TestFalse(TEXT("Stale stage dropped"), Quests->HasQuest(QuestId));
	}

	UGameplayStatics::DeleteGameInSlot(Slot, 0);
	Quest->QuestId = NAME_None;
	Token->ItemId = NAME_None;
	Reward->ItemId = NAME_None;
	return true;
}

#endif
