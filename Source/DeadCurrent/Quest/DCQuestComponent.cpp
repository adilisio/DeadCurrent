#include "Quest/DCQuestComponent.h"
#include "AI/DCScavengerCharacter.h"
#include "Combat/DCHealthComponent.h"
#include "EngineUtils.h"
#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Quest/DCQuestDefinition.h"

bool UDCQuestComponent::StartQuest(FName QuestId, FName StageId)
{
	if (QuestId.IsNone() || StageId.IsNone() || Stages.Contains(QuestId))
	{
		return false;
	}

	Stages.Add(QuestId, StageId);
	BroadcastIfLive(QuestId, StageId);
	return true;
}

bool UDCQuestComponent::SetStage(FName QuestId, FName StageId)
{
	if (QuestId.IsNone() || StageId.IsNone() || !Stages.Contains(QuestId))
	{
		return false;
	}

	if (Stages[QuestId] == StageId)
	{
		return true;
	}

	Stages[QuestId] = StageId;
	BroadcastIfLive(QuestId, StageId);
	return true;
}

bool UDCQuestComponent::CompleteQuest(FName QuestId)
{
	if (QuestId.IsNone())
	{
		return false;
	}

	const FName Done = CompletedStageId(QuestId);
	if (!Stages.Contains(QuestId))
	{
		return StartQuest(QuestId, Done);
	}

	return SetStage(QuestId, Done);
}

FName UDCQuestComponent::GetStage(FName QuestId) const
{
	const FName* Stage = Stages.Find(QuestId);
	return Stage ? *Stage : NAME_None;
}

bool UDCQuestComponent::IsComplete(FName QuestId) const
{
	const FName Stage = GetStage(QuestId);
	return !Stage.IsNone() && Stage == CompletedStageId(QuestId);
}

FText UDCQuestComponent::GetObjectiveText() const
{
	for (const TPair<FName, FName>& Pair : Stages)
	{
		if (IsComplete(Pair.Key))
		{
			continue;
		}

		if (const UDCQuestDefinition* Def = FindDefinition(Pair.Key))
		{
			if (const FDCQuestStage* Stage = Def->FindStage(Pair.Value))
			{
				if (!Stage->ObjectiveText.IsEmpty())
				{
					return Stage->ObjectiveText;
				}
			}
		}
	}
	return FText::GetEmpty();
}

void UDCQuestComponent::SetFlag(FName Flag)
{
	if (Flag.IsNone() || Flags.Contains(Flag))
	{
		return;
	}

	Flags.Add(Flag);
}

void UDCQuestComponent::NotifyHostileDied()
{
	TArray<FName> QuestIds;
	Stages.GetKeys(QuestIds);
	for (const FName QuestId : QuestIds)
	{
		const FName Current = GetStage(QuestId);
		const UDCQuestDefinition* Def = FindDefinition(QuestId);
		const FDCQuestStage* Stage = Def ? Def->FindStage(Current) : nullptr;
		if (Stage && Stage->bAdvanceOnHostileDeath && !Stage->NextStageOnHostileDeath.IsNone())
		{
			SetStage(QuestId, Stage->NextStageOnHostileDeath);
		}
	}
}

bool UDCQuestComponent::Meets(const FDCGameplayCondition& Condition) const
{
	bool bPass = true;
	switch (Condition.Type)
	{
	case EDCConditionType::None:
		bPass = true;
		break;
	case EDCConditionType::QuestStage:
		bPass = GetStage(Condition.Id) == Condition.Stage;
		break;
	case EDCConditionType::QuestNotStarted:
		bPass = !HasQuest(Condition.Id);
		break;
	case EDCConditionType::QuestActive:
		bPass = IsQuestActive(Condition.Id);
		break;
	case EDCConditionType::QuestComplete:
		bPass = IsComplete(Condition.Id);
		break;
	case EDCConditionType::HostileDead:
		bPass = IsHostileDead();
		break;
	case EDCConditionType::HasItem:
		bPass = HasItem(Condition.Id, FMath::Max(1, Condition.Quantity));
		break;
	case EDCConditionType::WorldFlag:
		bPass = HasFlag(Condition.Id);
		break;
	default:
		bPass = true;
		break;
	}

	return Condition.bNegate ? !bPass : bPass;
}

bool UDCQuestComponent::MeetsAll(const TArray<FDCGameplayCondition>& Conditions) const
{
	for (const FDCGameplayCondition& Condition : Conditions)
	{
		if (!Meets(Condition))
		{
			return false;
		}
	}
	return true;
}

void UDCQuestComponent::Apply(const FDCGameplayConsequence& Consequence)
{
	switch (Consequence.Type)
	{
	case EDCConsequenceType::StartQuest:
		StartQuest(Consequence.Id, Consequence.Stage);
		break;
	case EDCConsequenceType::SetQuestStage:
		SetStage(Consequence.Id, Consequence.Stage);
		break;
	case EDCConsequenceType::CompleteQuest:
		CompleteQuest(Consequence.Id);
		break;
	case EDCConsequenceType::GiveItem:
		GiveOrRemoveItem(Consequence.Id, Consequence.Item, Consequence.Quantity, true);
		break;
	case EDCConsequenceType::RemoveItem:
		GiveOrRemoveItem(Consequence.Id, Consequence.Item, Consequence.Quantity, false);
		break;
	case EDCConsequenceType::SetWorldFlag:
		SetFlag(Consequence.Id);
		break;
	default:
		break;
	}
}

void UDCQuestComponent::ApplyAll(const TArray<FDCGameplayConsequence>& Consequences)
{
	for (const FDCGameplayConsequence& Consequence : Consequences)
	{
		Apply(Consequence);
	}
}

void UDCQuestComponent::CaptureState(TArray<FDCSavedQuestState>& OutQuests, TArray<FName>& OutFlags) const
{
	OutQuests.Reset();
	for (const TPair<FName, FName>& Pair : Stages)
	{
		FDCSavedQuestState Saved;
		Saved.QuestId = Pair.Key;
		Saved.StageId = Pair.Value;
		OutQuests.Add(Saved);
	}
	OutFlags = Flags;
}

void UDCQuestComponent::ReplaceFromSaved(const TArray<FDCSavedQuestState>& SavedQuests, const TArray<FName>& SavedFlags)
{
	Stages.Empty();
	for (const FDCSavedQuestState& Saved : SavedQuests)
	{
		if (!Saved.QuestId.IsNone() && !Saved.StageId.IsNone())
		{
			Stages.Add(Saved.QuestId, Saved.StageId);
		}
	}
	Flags = SavedFlags;
}

FName UDCQuestComponent::CompletedStageId(FName QuestId) const
{
	if (const UDCQuestDefinition* Def = FindDefinition(QuestId))
	{
		return Def->CompletedStage.IsNone() ? FName(TEXT("done")) : Def->CompletedStage;
	}
	return TEXT("done");
}

const UDCQuestDefinition* UDCQuestComponent::FindDefinition(FName QuestId) const
{
	return UDCQuestDefinition::FindByQuestId(QuestId);
}

bool UDCQuestComponent::IsHostileDead() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	bool bSawAny = false;
	for (TActorIterator<ADCScavengerCharacter> It(World); It; ++It)
	{
		bSawAny = true;
		const UDCHealthComponent* Health = It->GetHealthComponent();
		if (Health && !Health->IsDead())
		{
			return false;
		}
	}
	return bSawAny;
}

bool UDCQuestComponent::HasItem(FName ItemId, int32 Quantity) const
{
	const AActor* Owner = GetOwner();
	const UDCInventoryComponent* Inventory = Owner
		? Owner->FindComponentByClass<UDCInventoryComponent>()
		: nullptr;
	return Inventory && Inventory->GetQuantityByItemId(ItemId) >= Quantity;
}

void UDCQuestComponent::GiveOrRemoveItem(FName ItemId, const TSoftObjectPtr<UDCItemDefinition>& Item, int32 Quantity, bool bGive)
{
	AActor* Owner = GetOwner();
	UDCInventoryComponent* Inventory = Owner
		? Owner->FindComponentByClass<UDCInventoryComponent>()
		: nullptr;
	if (!Inventory || Quantity <= 0)
	{
		return;
	}

	const UDCItemDefinition* Definition = Item.LoadSynchronous();
	if (!Definition)
	{
		Definition = UDCItemDefinition::FindByItemId(ItemId);
	}
	if (!Definition)
	{
		return;
	}

	if (bGive)
	{
		Inventory->AddItem(Definition, Quantity);
	}
	else
	{
		Inventory->RemoveItem(Definition, Quantity);
	}
}

void UDCQuestComponent::BroadcastIfLive(FName QuestId, FName StageId)
{
	OnQuestUpdated.Broadcast(QuestId, StageId);
}
