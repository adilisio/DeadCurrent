#include "Quest/DCQuestDefinition.h"
#include "UObject/UObjectIterator.h"

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

const UDCQuestDefinition* UDCQuestDefinition::FindByQuestId(FName QuestId)
{
	if (QuestId.IsNone())
	{
		return nullptr;
	}

	for (TObjectIterator<UDCQuestDefinition> It; It; ++It)
	{
		if (!It->HasAnyFlags(RF_ClassDefaultObject) && It->QuestId == QuestId)
		{
			return *It;
		}
	}
	return nullptr;
}
