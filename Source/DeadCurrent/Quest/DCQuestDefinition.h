#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DCQuestDefinition.generated.h"

USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCQuestStage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest")
	FName StageId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest")
	FText ObjectiveText;

	/** When a scavenger dies during this stage, move to NextStageOnHostileDeath. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest")
	bool bAdvanceOnHostileDeath = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest")
	FName NextStageOnHostileDeath;
};

/**
 *  One quest. Create under /Game/Quests as DA_Quest_<Name>.
 */
UCLASS(BlueprintType, Const)
class DEADCURRENT_API UDCQuestDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	FName QuestId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	FName CompletedStage = TEXT("done");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	TArray<FDCQuestStage> Stages;

	const FDCQuestStage* FindStage(FName StageId) const;

	static const UDCQuestDefinition* FindByQuestId(FName QuestId);
};
