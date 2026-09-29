#include "Core/DCGameplayRules.h"
#include "Core/DCTestHelpers.h"
#include "Combat/DCHealthComponent.h"
#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Quest/DCQuestComponent.h"
#include "Quest/DCQuestDefinition.h"
#include "Save/DCPersistentIdComponent.h"
#include "World/DCWorldStateSubsystem.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace DCRulesTest
{
	FDCGameplayCondition Cond(EDCConditionType Type, FName Id, FName Stage = NAME_None, int32 Quantity = 1, bool bNegate = false)
	{
		FDCGameplayCondition C;
		C.Type = Type;
		C.Id = Id;
		C.Stage = Stage;
		C.Quantity = Quantity;
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

	/** Two-stage quest: "open" -> "closed" (completes). */
	UDCQuestDefinition* MakeQuest(FName QuestId)
	{
		UDCQuestDefinition* Quest = NewObject<UDCQuestDefinition>();
		Quest->QuestId = QuestId;
		FDCQuestStage Open;
		Open.StageId = TEXT("open");
		FDCQuestStage Closed;
		Closed.StageId = TEXT("closed");
		Closed.bCompletesQuest = true;
		Quest->Stages = { Open, Closed };
		return Quest;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCRulesConditionsTest, "DeadCurrent.Rules.Conditions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCRulesConditionsTest::RunTest(const FString& Parameters)
{
	using namespace DCRulesTest;

	FDCTestWorld World;
	UDCItemDefinition* Item = NewObject<UDCItemDefinition>();
	Item->ItemId = TEXT("test.rules_item");
	Item->MaxStackSize = 10;
	UDCQuestDefinition* Quest = MakeQuest(TEXT("test.rules_quest"));

	AActor* Player = World.SpawnActor();
	UDCInventoryComponent* Inventory = FDCTestWorld::AddComponent<UDCInventoryComponent>(Player);
	UDCQuestComponent* Quests = FDCTestWorld::AddComponent<UDCQuestComponent>(Player);

	AActor* Target = World.SpawnActor();
	FDCTestWorld::AddComponent<UDCPersistentIdComponent>(Target, [](UDCPersistentIdComponent* C) { C->SetPersistentId(TEXT("test.target")); });
	UDCHealthComponent* TargetHealth = FDCTestWorld::AddComponent<UDCHealthComponent>(Target);

	const FDCRuleContext Context = FDCRuleContext::ForActor(Player);
	TestNotNull(TEXT("Context finds inventory"), Context.Inventory);
	TestNotNull(TEXT("Context finds quests"), Context.Quests);
	TestNotNull(TEXT("Context finds world state"), Context.WorldState);
	TestNotNull(TEXT("Context finds registry"), Context.Registry);

	auto Check = [&Context](const FDCGameplayCondition& C) { return UDCGameplayRules::CheckCondition(C, Context); };

	// None and empty lists pass.
	TestTrue(TEXT("None passes"), Check(FDCGameplayCondition()));
	TestTrue(TEXT("Empty list passes"), UDCGameplayRules::CheckConditions({}, Context));

	// HasItem honours quantity.
	TestFalse(TEXT("HasItem without item"), Check(Cond(EDCConditionType::HasItem, Item->ItemId)));
	Inventory->AddItem(Item, 2);
	TestTrue(TEXT("HasItem with 2"), Check(Cond(EDCConditionType::HasItem, Item->ItemId)));
	TestTrue(TEXT("HasItem x2"), Check(Cond(EDCConditionType::HasItem, Item->ItemId, NAME_None, 2)));
	TestFalse(TEXT("HasItem x3 fails"), Check(Cond(EDCConditionType::HasItem, Item->ItemId, NAME_None, 3)));
	TestTrue(TEXT("Negated HasItem x3 passes"), Check(Cond(EDCConditionType::HasItem, Item->ItemId, NAME_None, 3, true)));

	// Quest states.
	TestTrue(TEXT("QuestNotStarted"), Check(Cond(EDCConditionType::QuestNotStarted, Quest->QuestId)));
	TestFalse(TEXT("QuestActive before start"), Check(Cond(EDCConditionType::QuestActive, Quest->QuestId)));
	TestTrue(TEXT("Start quest"), Quests->StartQuest(Quest->QuestId));
	TestFalse(TEXT("QuestNotStarted after start"), Check(Cond(EDCConditionType::QuestNotStarted, Quest->QuestId)));
	TestTrue(TEXT("QuestActive"), Check(Cond(EDCConditionType::QuestActive, Quest->QuestId)));
	TestTrue(TEXT("QuestStage open"), Check(Cond(EDCConditionType::QuestStage, Quest->QuestId, TEXT("open"))));
	TestFalse(TEXT("QuestStage closed"), Check(Cond(EDCConditionType::QuestStage, Quest->QuestId, TEXT("closed"))));
	TestFalse(TEXT("QuestStage with no stage never passes"), Check(Cond(EDCConditionType::QuestStage, Quest->QuestId)));
	TestFalse(TEXT("QuestComplete while open"), Check(Cond(EDCConditionType::QuestComplete, Quest->QuestId)));
	Quests->SetStage(Quest->QuestId, TEXT("closed"));
	TestTrue(TEXT("QuestComplete"), Check(Cond(EDCConditionType::QuestComplete, Quest->QuestId)));
	TestFalse(TEXT("QuestActive after complete"), Check(Cond(EDCConditionType::QuestActive, Quest->QuestId)));

	// World flags.
	TestFalse(TEXT("WorldFlag unset"), Check(Cond(EDCConditionType::WorldFlag, TEXT("test.flag"))));
	Context.WorldState->SetFlag(TEXT("test.flag"));
	TestTrue(TEXT("WorldFlag set"), Check(Cond(EDCConditionType::WorldFlag, TEXT("test.flag"))));
	TestFalse(TEXT("Negated WorldFlag"), Check(Cond(EDCConditionType::WorldFlag, TEXT("test.flag"), NAME_None, 1, true)));

	// ActorDead reads the persistent actor's health.
	TestFalse(TEXT("ActorDead while alive"), Check(Cond(EDCConditionType::ActorDead, TEXT("test.target"))));
	TestFalse(TEXT("ActorDead for unknown id"), Check(Cond(EDCConditionType::ActorDead, TEXT("test.nobody"))));
	FDCDamageInfo Damage;
	Damage.Amount = TargetHealth->GetMaxHealth();
	TargetHealth->ApplyDamage(Damage);
	TestTrue(TEXT("ActorDead after lethal damage"), Check(Cond(EDCConditionType::ActorDead, TEXT("test.target"))));

	// LocationDiscovered reads the world's discovered locations.
	TestFalse(TEXT("LocationDiscovered before"), Check(Cond(EDCConditionType::LocationDiscovered, TEXT("test.place"))));
	Context.WorldState->DiscoverLocation(TEXT("test.place"));
	TestTrue(TEXT("LocationDiscovered after"), Check(Cond(EDCConditionType::LocationDiscovered, TEXT("test.place"))));
	TestFalse(TEXT("Negated LocationDiscovered"), Check(Cond(EDCConditionType::LocationDiscovered, TEXT("test.place"), NAME_None, 1, true)));
	TestFalse(TEXT("A flag is not a location"), Check(Cond(EDCConditionType::LocationDiscovered, TEXT("test.flag"))));

	// Lists are ANDed.
	TestTrue(TEXT("All pass"), UDCGameplayRules::CheckConditions({
		Cond(EDCConditionType::WorldFlag, TEXT("test.flag")),
		Cond(EDCConditionType::ActorDead, TEXT("test.target")) }, Context));
	TestFalse(TEXT("One fails"), UDCGameplayRules::CheckConditions({
		Cond(EDCConditionType::WorldFlag, TEXT("test.flag")),
		Cond(EDCConditionType::WorldFlag, TEXT("test.other")) }, Context));

	// A context with nothing in it fails safely.
	const FDCRuleContext Empty;
	TestFalse(TEXT("Empty context HasItem"), UDCGameplayRules::CheckCondition(Cond(EDCConditionType::HasItem, Item->ItemId), Empty));
	TestFalse(TEXT("Empty context WorldFlag"), UDCGameplayRules::CheckCondition(Cond(EDCConditionType::WorldFlag, TEXT("test.flag")), Empty));
	TestTrue(TEXT("Empty context QuestNotStarted"), UDCGameplayRules::CheckCondition(Cond(EDCConditionType::QuestNotStarted, Quest->QuestId), Empty));

	Quest->QuestId = NAME_None;
	Item->ItemId = NAME_None;
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCRulesConsequencesTest, "DeadCurrent.Rules.Consequences",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCRulesConsequencesTest::RunTest(const FString& Parameters)
{
	using namespace DCRulesTest;

	FDCTestWorld World;
	UDCItemDefinition* Item = NewObject<UDCItemDefinition>();
	Item->ItemId = TEXT("test.rules_reward");
	Item->MaxStackSize = 50;
	UDCQuestDefinition* Quest = MakeQuest(TEXT("test.rules_consequence_quest"));

	AActor* Player = World.SpawnActor();
	UDCInventoryComponent* Inventory = FDCTestWorld::AddComponent<UDCInventoryComponent>(Player);
	UDCQuestComponent* Quests = FDCTestWorld::AddComponent<UDCQuestComponent>(Player);
	const FDCRuleContext Context = FDCRuleContext::ForActor(Player);

	// Items by id and by direct reference.
	UDCGameplayRules::ApplyConsequence(Cons(EDCConsequenceType::GiveItem, Item->ItemId, NAME_None, 5), Context);
	TestEqual(TEXT("GiveItem by id"), Inventory->GetQuantity(Item), 5);
	FDCGameplayConsequence GiveRef = Cons(EDCConsequenceType::GiveItem, NAME_None, NAME_None, 2);
	GiveRef.Item = Item;
	UDCGameplayRules::ApplyConsequence(GiveRef, Context);
	TestEqual(TEXT("GiveItem by reference"), Inventory->GetQuantity(Item), 7);
	UDCGameplayRules::ApplyConsequence(Cons(EDCConsequenceType::RemoveItem, Item->ItemId, NAME_None, 3), Context);
	TestEqual(TEXT("RemoveItem"), Inventory->GetQuantity(Item), 4);
	AddExpectedError(TEXT("unknown item"), EAutomationExpectedErrorFlags::Contains, 1);
	UDCGameplayRules::ApplyConsequence(Cons(EDCConsequenceType::GiveItem, TEXT("test.no_such_item")), Context);
	TestEqual(TEXT("Unknown item changes nothing"), Inventory->GetStacks().Num(), 1);

	// Quests: start at the definition's first stage, then move.
	UDCGameplayRules::ApplyConsequence(Cons(EDCConsequenceType::StartQuest, Quest->QuestId), Context);
	TestEqual(TEXT("StartQuest uses start stage"), Quests->GetStage(Quest->QuestId), FName(TEXT("open")));
	UDCGameplayRules::ApplyConsequence(Cons(EDCConsequenceType::SetQuestStage, Quest->QuestId, TEXT("closed")), Context);
	TestTrue(TEXT("SetQuestStage to outcome completes"), Quests->IsComplete(Quest->QuestId));

	// World flags.
	UDCGameplayRules::ApplyConsequence(Cons(EDCConsequenceType::SetWorldFlag, TEXT("test.cons_flag")), Context);
	TestTrue(TEXT("SetWorldFlag"), Context.WorldState->HasFlag(TEXT("test.cons_flag")));
	UDCGameplayRules::ApplyConsequence(Cons(EDCConsequenceType::ClearWorldFlag, TEXT("test.cons_flag")), Context);
	TestFalse(TEXT("ClearWorldFlag"), Context.WorldState->HasFlag(TEXT("test.cons_flag")));

	// Lists apply in order: the flag set first is visible to nothing else here, but both land.
	UDCGameplayRules::ApplyConsequences({
		Cons(EDCConsequenceType::SetWorldFlag, TEXT("test.a")),
		Cons(EDCConsequenceType::SetWorldFlag, TEXT("test.b")) }, Context);
	TestEqual(TEXT("Flags in order"), Context.WorldState->GetFlags(), TArray<FName>({ TEXT("test.a"), TEXT("test.b") }));

	// Missing targets are ignored rather than crashing.
	UDCGameplayRules::ApplyConsequences({
		Cons(EDCConsequenceType::SetWorldFlag, TEXT("test.c")),
		Cons(EDCConsequenceType::StartQuest, TEXT("test.q")) }, FDCRuleContext());

	Quest->QuestId = NAME_None;
	Item->ItemId = NAME_None;
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCRulesValidationTest, "DeadCurrent.Rules.Validation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCRulesValidationTest::RunTest(const FString& Parameters)
{
	using namespace DCRulesTest;

	UDCQuestDefinition* Quest = MakeQuest(TEXT("test.valid_quest"));
	UDCItemDefinition* Item = NewObject<UDCItemDefinition>();
	Item->ItemId = TEXT("test.valid_item");

	auto ProblemsFor = [](const FDCGameplayCondition& C) { TArray<FString> P; UDCGameplayRules::ValidateReferences(C, P); return P.Num(); };
	auto ProblemsForCons = [](const FDCGameplayConsequence& C) { TArray<FString> P; UDCGameplayRules::ValidateReferences(C, P); return P.Num(); };

	TestEqual(TEXT("Good quest stage"), ProblemsFor(Cond(EDCConditionType::QuestStage, Quest->QuestId, TEXT("open"))), 0);
	TestEqual(TEXT("Typo in stage"), ProblemsFor(Cond(EDCConditionType::QuestStage, Quest->QuestId, TEXT("opne"))), 1);
	TestEqual(TEXT("Unknown quest"), ProblemsFor(Cond(EDCConditionType::QuestActive, TEXT("test.no_quest"))), 1);
	TestEqual(TEXT("Good item"), ProblemsFor(Cond(EDCConditionType::HasItem, Item->ItemId)), 0);
	TestEqual(TEXT("Unknown item"), ProblemsFor(Cond(EDCConditionType::HasItem, TEXT("test.no_item"))), 1);
	TestEqual(TEXT("Flag needs an id"), ProblemsFor(Cond(EDCConditionType::WorldFlag, NAME_None)), 1);
	TestEqual(TEXT("None type flagged"), ProblemsFor(FDCGameplayCondition()), 1);
	TestEqual(TEXT("SetQuestStage bad stage"), ProblemsForCons(Cons(EDCConsequenceType::SetQuestStage, Quest->QuestId, TEXT("nope"))), 1);
	TestEqual(TEXT("SetQuestStage without stage"), ProblemsForCons(Cons(EDCConsequenceType::SetQuestStage, Quest->QuestId)), 1);
	TestEqual(TEXT("StartQuest default stage ok"), ProblemsForCons(Cons(EDCConsequenceType::StartQuest, Quest->QuestId)), 0);
	TestEqual(TEXT("GiveItem unknown"), ProblemsForCons(Cons(EDCConsequenceType::GiveItem, TEXT("test.no_item"))), 1);

	Quest->QuestId = NAME_None;
	Item->ItemId = NAME_None;
	return true;
}

#endif
