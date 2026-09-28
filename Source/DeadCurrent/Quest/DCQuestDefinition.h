#pragma once

#include "CoreMinimal.h"
#include "Core/DCGameplayTypes.h"
#include "Engine/DataAsset.h"
#include "DCQuestDefinition.generated.h"

/**
 *  A way out of a stage: when every condition passes, the quest moves to NextStage.
 *  Transitions are checked in order whenever something conditions can read changes
 *  (inventory, world flags, deaths, quest stages).
 */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCQuestTransition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest")
	TArray<FDCGameplayCondition> Conditions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest")
	FName NextStage;
};

/**
 *  One step of a quest. A stage with bCompletesQuest is an outcome; a quest may have several.
 */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCQuestStage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest")
	FName StageId;

	/** Shown on the HUD while this stage is current. For a completing stage, the journal summary of that outcome. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest", meta=(MultiLine="true"))
	FText ObjectiveText;

	/** Reaching this stage completes the quest with this outcome. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest")
	bool bCompletesQuest = false;

	/** Applied once when the quest enters this stage (not on save restore). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest")
	TArray<FDCGameplayConsequence> OnEnter;

	/** Checked in order; the first whose conditions pass moves the quest on. Ignored on completing stages. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest")
	TArray<FDCQuestTransition> Transitions;
};

/**
 *  One quest. Create under /Game/Quests as DA_Quest_<Name>. QuestId and StageIds are stored
 *  in saves: never rename them once shipped.
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

	/** Stage a quest starts at when no stage is given. Empty means the first stage. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	FName StartStage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quest")
	TArray<FDCQuestStage> Stages;

	const FDCQuestStage* FindStage(FName StageId) const;

	FName GetStartStage() const;

	bool IsCompletingStage(FName StageId) const;

	/** Finds a loaded quest definition. UDCContentSubsystem keeps every quest under /Game/Quests loaded. */
	static const UDCQuestDefinition* FindByQuestId(FName QuestId);

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
