#include "Quest/DCQuestComponent.h"
#include "Core/DCGameplayRules.h"
#include "DeadCurrent.h"
#include "Inventory/DCInventoryComponent.h"
#include "Quest/DCQuestDefinition.h"
#include "World/DCWorldStateSubsystem.h"

/** Transition chains longer than this are treated as a content loop. */
static constexpr int32 DCMaxQuestEvaluationPasses = 16;

void UDCQuestComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UDCInventoryComponent* Inventory = GetOwner() ? GetOwner()->FindComponentByClass<UDCInventoryComponent>() : nullptr)
	{
		Inventory->OnInventoryChanged.AddDynamic(this, &UDCQuestComponent::HandleInventoryChanged);
	}

	if (UDCWorldStateSubsystem* WorldState = UDCWorldStateSubsystem::Get(this))
	{
		WorldStateHandle = WorldState->OnChanged.AddUObject(this, &UDCQuestComponent::HandleWorldStateChanged);
	}
}

void UDCQuestComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UDCWorldStateSubsystem* WorldState = UDCWorldStateSubsystem::Get(this))
	{
		WorldState->OnChanged.Remove(WorldStateHandle);
	}
	WorldStateHandle.Reset();

	Super::EndPlay(EndPlayReason);
}

bool UDCQuestComponent::StartQuest(FName QuestId, FName StageId)
{
	if (QuestId.IsNone() || HasQuest(QuestId))
	{
		return false;
	}

	const UDCQuestDefinition* Definition = UDCQuestDefinition::FindByQuestId(QuestId);
	if (StageId.IsNone() && Definition)
	{
		StageId = Definition->GetStartStage();
	}

	if (StageId.IsNone() || (Definition && !Definition->FindStage(StageId)))
	{
		UE_LOG(LogDeadCurrent, Warning, TEXT("[DCQUEST] cannot start %s at stage '%s'%s"), *QuestId.ToString(),
			*StageId.ToString(), Definition ? TEXT("") : TEXT(" (no definition loaded)"));
		return false;
	}

	FDCQuestProgress& Progress = Quests.AddDefaulted_GetRef();
	Progress.QuestId = QuestId;
	UE_LOG(LogDeadCurrent, Log, TEXT("[DCQUEST] start %s"), *QuestId.ToString());
	EnterStage(QuestId, StageId);
	return true;
}

bool UDCQuestComponent::SetStage(FName QuestId, FName StageId)
{
	const FDCQuestProgress* Progress = FindProgress(QuestId);
	if (!Progress || StageId.IsNone())
	{
		return false;
	}

	const UDCQuestDefinition* Definition = UDCQuestDefinition::FindByQuestId(QuestId);
	if (Definition && !Definition->FindStage(StageId))
	{
		UE_LOG(LogDeadCurrent, Warning, TEXT("[DCQUEST] %s has no stage '%s'"), *QuestId.ToString(), *StageId.ToString());
		return false;
	}

	if (Progress->StageId != StageId)
	{
		EnterStage(QuestId, StageId);
	}
	return true;
}

FName UDCQuestComponent::GetStage(FName QuestId) const
{
	const FDCQuestProgress* Progress = FindProgress(QuestId);
	return Progress ? Progress->StageId : NAME_None;
}

EDCQuestStatus UDCQuestComponent::GetQuestStatus(FName QuestId) const
{
	const FDCQuestProgress* Progress = FindProgress(QuestId);
	if (!Progress)
	{
		return EDCQuestStatus::NotStarted;
	}

	const FDCQuestStage* Stage = FindCurrentStage(*Progress);
	return Stage && Stage->bCompletesQuest ? EDCQuestStatus::Complete : EDCQuestStatus::Active;
}

FText UDCQuestComponent::GetObjectiveText() const
{
	const FName QuestId = GetTrackedQuestId();
	return QuestId.IsNone() ? FText::GetEmpty() : GetStageText(QuestId);
}

FName UDCQuestComponent::GetTrackedQuestId() const
{
	for (const FDCQuestProgress& Progress : Quests)
	{
		const FDCQuestStage* Stage = FindCurrentStage(Progress);
		if (Stage && !Stage->bCompletesQuest && !Stage->ObjectiveText.IsEmpty())
		{
			return Progress.QuestId;
		}
	}
	return NAME_None;
}

FText UDCQuestComponent::GetStageText(FName QuestId) const
{
	const FDCQuestProgress* Progress = FindProgress(QuestId);
	const FDCQuestStage* Stage = Progress ? FindCurrentStage(*Progress) : nullptr;
	return Stage ? Stage->ObjectiveText : FText::GetEmpty();
}

void UDCQuestComponent::EvaluateQuests()
{
	if (bEvaluating)
	{
		bEvaluateAgain = true;
		return;
	}

	TGuardValue<bool> Guard(bEvaluating, true);
	for (int32 Pass = 0; Pass < DCMaxQuestEvaluationPasses; ++Pass)
	{
		bEvaluateAgain = false;
		bool bMoved = false;
		const FDCRuleContext Context = MakeRuleContext();

		// Copy: entering a stage can start other quests and grow the log.
		const TArray<FDCQuestProgress> Snapshot = Quests;
		for (const FDCQuestProgress& Progress : Snapshot)
		{
			const FDCQuestStage* Stage = FindCurrentStage(Progress);
			if (!Stage || Stage->bCompletesQuest)
			{
				continue;
			}

			for (const FDCQuestTransition& Transition : Stage->Transitions)
			{
				if (UDCGameplayRules::CheckConditions(Transition.Conditions, Context))
				{
					EnterStage(Progress.QuestId, Transition.NextStage);
					bMoved = true;
					break;
				}
			}
		}

		if (!bMoved && !bEvaluateAgain)
		{
			return;
		}
	}

	UE_LOG(LogDeadCurrent, Warning, TEXT("[DCQUEST] transitions still changing after %d passes; check quest data for loops"),
		DCMaxQuestEvaluationPasses);
}

void UDCQuestComponent::CaptureState(TArray<FDCSavedQuestState>& OutQuests) const
{
	OutQuests.Reset();
	for (const FDCQuestProgress& Progress : Quests)
	{
		FDCSavedQuestState& Saved = OutQuests.AddDefaulted_GetRef();
		Saved.QuestId = Progress.QuestId;
		Saved.StageId = Progress.StageId;
	}
}

void UDCQuestComponent::ReplaceFromSaved(const TArray<FDCSavedQuestState>& SavedQuests)
{
	Quests.Reset();
	for (const FDCSavedQuestState& Saved : SavedQuests)
	{
		if (Saved.QuestId.IsNone() || Saved.StageId.IsNone() || HasQuest(Saved.QuestId))
		{
			continue;
		}

		// A stage that no longer exists (quest data changed since the save) would leave the quest stuck.
		// Drop it so the quest can be picked up again.
		const UDCQuestDefinition* Definition = UDCQuestDefinition::FindByQuestId(Saved.QuestId);
		if (Definition && !Definition->FindStage(Saved.StageId))
		{
			UE_LOG(LogDeadCurrent, Warning, TEXT("[DCQUEST] saved stage %s.%s no longer exists; quest reset"),
				*Saved.QuestId.ToString(), *Saved.StageId.ToString());
			continue;
		}

		FDCQuestProgress& Progress = Quests.AddDefaulted_GetRef();
		Progress.QuestId = Saved.QuestId;
		Progress.StageId = Saved.StageId;
	}
}

FDCQuestProgress* UDCQuestComponent::FindProgress(FName QuestId)
{
	return Quests.FindByPredicate([QuestId](const FDCQuestProgress& P) { return P.QuestId == QuestId; });
}

const FDCQuestProgress* UDCQuestComponent::FindProgress(FName QuestId) const
{
	if (QuestId.IsNone())
	{
		return nullptr;
	}
	return Quests.FindByPredicate([QuestId](const FDCQuestProgress& P) { return P.QuestId == QuestId; });
}

const FDCQuestStage* UDCQuestComponent::FindCurrentStage(const FDCQuestProgress& Progress) const
{
	const UDCQuestDefinition* Definition = UDCQuestDefinition::FindByQuestId(Progress.QuestId);
	return Definition ? Definition->FindStage(Progress.StageId) : nullptr;
}

FDCRuleContext UDCQuestComponent::MakeRuleContext()
{
	FDCRuleContext Context = FDCRuleContext::ForActor(GetOwner());
	Context.Quests = this;
	return Context;
}

void UDCQuestComponent::EnterStage(FName QuestId, FName StageId)
{
	FDCQuestProgress* Progress = FindProgress(QuestId);
	if (!Progress)
	{
		return;
	}

	Progress->StageId = StageId;
	UE_LOG(LogDeadCurrent, Log, TEXT("[DCQUEST] %s -> %s"), *QuestId.ToString(), *StageId.ToString());
	OnQuestUpdated.Broadcast(QuestId, StageId);

	const UDCQuestDefinition* Definition = UDCQuestDefinition::FindByQuestId(QuestId);
	if (const FDCQuestStage* Stage = Definition ? Definition->FindStage(StageId) : nullptr)
	{
		UDCGameplayRules::ApplyConsequences(Stage->OnEnter, MakeRuleContext());
	}

	EvaluateQuests();
}

void UDCQuestComponent::HandleInventoryChanged(UDCInventoryComponent* Inventory)
{
	EvaluateQuests();
}

void UDCQuestComponent::HandleWorldStateChanged()
{
	EvaluateQuests();
}
