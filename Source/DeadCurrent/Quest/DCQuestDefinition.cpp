#include "Quest/DCQuestDefinition.h"
#include "UObject/UObjectIterator.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "DCQuestDefinition"

const FDCQuestStage* UDCQuestDefinition::FindStage(FName StageId) const
{
	if (StageId.IsNone())
	{
		return nullptr;
	}

	for (const FDCQuestStage& Stage : Stages)
	{
		if (Stage.StageId == StageId)
		{
			return &Stage;
		}
	}
	return nullptr;
}

FName UDCQuestDefinition::GetStartStage() const
{
	if (!StartStage.IsNone())
	{
		return StartStage;
	}
	return Stages.Num() > 0 ? Stages[0].StageId : NAME_None;
}

bool UDCQuestDefinition::IsCompletingStage(FName StageId) const
{
	const FDCQuestStage* Stage = FindStage(StageId);
	return Stage && Stage->bCompletesQuest;
}

const UDCQuestDefinition* UDCQuestDefinition::FindByQuestId(FName QuestId)
{
	if (QuestId.IsNone())
	{
		return nullptr;
	}

	for (TObjectIterator<UDCQuestDefinition> It; It; ++It)
	{
		if (!It->HasAnyFlags(RF_ClassDefaultObject | RF_BeginDestroyed | RF_FinishDestroyed) && It->QuestId == QuestId)
		{
			return *It;
		}
	}
	return nullptr;
}

#if WITH_EDITOR
EDataValidationResult UDCQuestDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (QuestId.IsNone())
	{
		Context.AddError(LOCTEXT("NoId", "QuestId is empty."));
		Result = EDataValidationResult::Invalid;
	}

	if (!StartStage.IsNone() && !FindStage(StartStage))
	{
		Context.AddError(FText::Format(LOCTEXT("BadStart", "StartStage {0} does not exist."), FText::FromName(StartStage)));
		Result = EDataValidationResult::Invalid;
	}

	bool bHasOutcome = false;
	TSet<FName> Seen;
	for (const FDCQuestStage& Stage : Stages)
	{
		bHasOutcome |= Stage.bCompletesQuest;
		if (Stage.StageId.IsNone() || Seen.Contains(Stage.StageId))
		{
			Context.AddError(FText::Format(LOCTEXT("BadStageId", "Stage id '{0}' is empty or duplicated."), FText::FromName(Stage.StageId)));
			Result = EDataValidationResult::Invalid;
		}
		Seen.Add(Stage.StageId);

		for (const FDCQuestTransition& Transition : Stage.Transitions)
		{
			if (!FindStage(Transition.NextStage))
			{
				Context.AddError(FText::Format(LOCTEXT("BadNext", "Stage {0} has a transition to missing stage {1}."),
					FText::FromName(Stage.StageId), FText::FromName(Transition.NextStage)));
				Result = EDataValidationResult::Invalid;
			}
		}
	}

	if (!bHasOutcome)
	{
		Context.AddError(LOCTEXT("NoOutcome", "No stage completes the quest."));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
