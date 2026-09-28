#include "Core/DCGameplayRules.h"
#include "Combat/DCHealthComponent.h"
#include "DeadCurrent.h"
#include "Engine/World.h"
#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Quest/DCQuestComponent.h"
#include "Save/DCPersistentRegistry.h"
#include "World/DCWorldStateSubsystem.h"

FDCRuleContext FDCRuleContext::ForActor(AActor* Instigator)
{
	FDCRuleContext Context;
	Context.Instigator = Instigator;
	if (!Instigator)
	{
		return Context;
	}

	Context.Inventory = Instigator->FindComponentByClass<UDCInventoryComponent>();
	Context.Quests = Instigator->FindComponentByClass<UDCQuestComponent>();
	if (UWorld* World = Instigator->GetWorld())
	{
		Context.WorldState = World->GetSubsystem<UDCWorldStateSubsystem>();
		Context.Registry = World->GetSubsystem<UDCPersistentRegistry>();
	}
	return Context;
}

static bool DCIsActorDead(FName PersistentId, const FDCRuleContext& Context)
{
	const AActor* Actor = Context.Registry ? Context.Registry->FindActor(PersistentId) : nullptr;
	const UDCHealthComponent* Health = Actor ? Actor->FindComponentByClass<UDCHealthComponent>() : nullptr;
	return Health && Health->IsDead();
}

bool UDCGameplayRules::CheckCondition(const FDCGameplayCondition& Condition, const FDCRuleContext& Context)
{
	const UDCQuestComponent* Quests = Context.Quests;

	bool bPass = true;
	switch (Condition.Type)
	{
	case EDCConditionType::None:
		bPass = true;
		break;
	case EDCConditionType::HasItem:
		bPass = Context.Inventory
			&& Context.Inventory->GetQuantityByItemId(Condition.Id) >= FMath::Max(1, Condition.Quantity);
		break;
	case EDCConditionType::QuestNotStarted:
		bPass = !Quests || !Quests->HasQuest(Condition.Id);
		break;
	case EDCConditionType::QuestActive:
		bPass = Quests && Quests->IsQuestActive(Condition.Id);
		break;
	case EDCConditionType::QuestComplete:
		bPass = Quests && Quests->IsComplete(Condition.Id);
		break;
	case EDCConditionType::QuestStage:
		bPass = Quests && !Condition.Stage.IsNone() && Quests->GetStage(Condition.Id) == Condition.Stage;
		break;
	case EDCConditionType::WorldFlag:
		bPass = Context.WorldState && Context.WorldState->HasFlag(Condition.Id);
		break;
	case EDCConditionType::ActorDead:
		bPass = DCIsActorDead(Condition.Id, Context);
		break;
	default:
		UE_LOG(LogDeadCurrent, Warning, TEXT("[DCRULES] unhandled condition type %d"), static_cast<int32>(Condition.Type));
		bPass = false;
		break;
	}

	return Condition.bNegate ? !bPass : bPass;
}

bool UDCGameplayRules::CheckConditions(const TArray<FDCGameplayCondition>& Conditions, const FDCRuleContext& Context)
{
	for (const FDCGameplayCondition& Condition : Conditions)
	{
		if (!CheckCondition(Condition, Context))
		{
			return false;
		}
	}
	return true;
}

static const UDCItemDefinition* DCResolveItem(const FDCGameplayConsequence& Consequence)
{
	if (const UDCItemDefinition* Item = Consequence.Item.LoadSynchronous())
	{
		return Item;
	}
	return UDCItemDefinition::FindByItemId(Consequence.Id);
}

void UDCGameplayRules::ApplyConsequence(const FDCGameplayConsequence& Consequence, const FDCRuleContext& Context)
{
	UE_LOG(LogDeadCurrent, Log, TEXT("[DCRULES] apply %s"), *Describe(Consequence));

	switch (Consequence.Type)
	{
	case EDCConsequenceType::None:
		break;
	case EDCConsequenceType::GiveItem:
	case EDCConsequenceType::RemoveItem:
	{
		const UDCItemDefinition* Item = DCResolveItem(Consequence);
		if (!Context.Inventory || !Item || Consequence.Quantity <= 0)
		{
			UE_LOG(LogDeadCurrent, Warning, TEXT("[DCRULES] %s skipped: %s"), *Describe(Consequence),
				!Item ? TEXT("unknown item") : TEXT("no inventory"));
			break;
		}

		if (Consequence.Type == EDCConsequenceType::GiveItem)
		{
			Context.Inventory->AddItem(Item, Consequence.Quantity);
		}
		else
		{
			Context.Inventory->RemoveItem(Item, Consequence.Quantity);
		}
		break;
	}
	case EDCConsequenceType::StartQuest:
		if (Context.Quests)
		{
			Context.Quests->StartQuest(Consequence.Id, Consequence.Stage);
		}
		break;
	case EDCConsequenceType::SetQuestStage:
		if (Context.Quests)
		{
			Context.Quests->SetStage(Consequence.Id, Consequence.Stage);
		}
		break;
	case EDCConsequenceType::SetWorldFlag:
		if (Context.WorldState)
		{
			Context.WorldState->SetFlag(Consequence.Id);
		}
		break;
	case EDCConsequenceType::ClearWorldFlag:
		if (Context.WorldState)
		{
			Context.WorldState->ClearFlag(Consequence.Id);
		}
		break;
	default:
		UE_LOG(LogDeadCurrent, Warning, TEXT("[DCRULES] unhandled consequence type %d"), static_cast<int32>(Consequence.Type));
		break;
	}
}

void UDCGameplayRules::ApplyConsequences(const TArray<FDCGameplayConsequence>& Consequences, const FDCRuleContext& Context)
{
	for (const FDCGameplayConsequence& Consequence : Consequences)
	{
		ApplyConsequence(Consequence, Context);
	}
}

bool UDCGameplayRules::CheckConditionsFor(AActor* Instigator, const TArray<FDCGameplayCondition>& Conditions)
{
	return CheckConditions(Conditions, FDCRuleContext::ForActor(Instigator));
}

void UDCGameplayRules::ApplyConsequencesFor(AActor* Instigator, const TArray<FDCGameplayConsequence>& Consequences)
{
	ApplyConsequences(Consequences, FDCRuleContext::ForActor(Instigator));
}

FString UDCGameplayRules::Describe(const FDCGameplayCondition& Condition)
{
	const FString Type = StaticEnum<EDCConditionType>()->GetNameStringByValue(static_cast<int64>(Condition.Type));
	FString Text = FString::Printf(TEXT("%s%s(%s"), Condition.bNegate ? TEXT("!") : TEXT(""), *Type, *Condition.Id.ToString());
	if (Condition.Type == EDCConditionType::QuestStage)
	{
		Text += TEXT(", ") + Condition.Stage.ToString();
	}
	else if (Condition.Type == EDCConditionType::HasItem && Condition.Quantity > 1)
	{
		Text += FString::Printf(TEXT(", %d"), Condition.Quantity);
	}
	return Text + TEXT(")");
}

FString UDCGameplayRules::Describe(const FDCGameplayConsequence& Consequence)
{
	const FString Type = StaticEnum<EDCConsequenceType>()->GetNameStringByValue(static_cast<int64>(Consequence.Type));
	FString Text = FString::Printf(TEXT("%s(%s"), *Type, *Consequence.Id.ToString());
	if (!Consequence.Stage.IsNone())
	{
		Text += TEXT(", ") + Consequence.Stage.ToString();
	}
	if (Consequence.Type == EDCConsequenceType::GiveItem || Consequence.Type == EDCConsequenceType::RemoveItem)
	{
		Text += FString::Printf(TEXT(", %d"), Consequence.Quantity);
	}
	return Text + TEXT(")");
}
