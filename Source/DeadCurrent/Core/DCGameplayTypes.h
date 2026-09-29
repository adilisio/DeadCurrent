#pragma once

#include "CoreMinimal.h"
#include "DCGameplayTypes.generated.h"

class UDCItemDefinition;

/**
 *  What a condition checks. Dialogue, quests and interactables all use this one list.
 *  New checks (skills, attributes, reputation, discovered information) are added here
 *  and in UDCGameplayRules::CheckCondition.
 */
UENUM(BlueprintType)
enum class EDCConditionType : uint8
{
	None UMETA(Hidden),
	/** The instigator carries at least Quantity of item Id. */
	HasItem,
	/** Quest Id has not been started. */
	QuestNotStarted,
	/** Quest Id is started and has not reached a completing stage. */
	QuestActive,
	/** Quest Id reached any completing stage (any outcome). */
	QuestComplete,
	/** Quest Id is at stage Stage. */
	QuestStage,
	/** World flag Id is set. */
	WorldFlag,
	/** The actor with persistent id Id is dead. */
	ActorDead,
	/** Location Id has been discovered (UDCWorldStateSubsystem). */
	LocationDiscovered
};

/**
 *  What a consequence does. Shared by dialogue choices, quest stages and interactables.
 */
UENUM(BlueprintType)
enum class EDCConsequenceType : uint8
{
	None UMETA(Hidden),
	/** Add Quantity of item Id (or Item) to the instigator's inventory. */
	GiveItem,
	/** Remove Quantity of item Id (or Item) from the instigator's inventory. */
	RemoveItem,
	/** Start quest Id at Stage, or at the quest's start stage when Stage is empty. */
	StartQuest,
	/** Move quest Id to Stage. Moving to a completing stage completes the quest. */
	SetQuestStage,
	/** Set world flag Id. */
	SetWorldFlag,
	/** Clear world flag Id. */
	ClearWorldFlag
};

/**
 *  One check. A list of conditions passes when every entry passes.
 */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCGameplayCondition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition")
	EDCConditionType Type = EDCConditionType::None;

	/** Item id, quest id, world flag, persistent actor id, or location id, depending on Type. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition",
		meta=(EditCondition="Type==EDCConditionType::QuestStage", EditConditionHides))
	FName Stage;

	/** Minimum quantity for HasItem. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition",
		meta=(ClampMin="1", EditCondition="Type==EDCConditionType::HasItem", EditConditionHides))
	int32 Quantity = 1;

	/** Pass when the check fails instead. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition")
	bool bNegate = false;
};

/**
 *  One change to the game. Lists run in order.
 */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCGameplayConsequence
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Consequence")
	EDCConsequenceType Type = EDCConsequenceType::None;

	/** Item id, quest id, or world flag, depending on Type. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Consequence")
	FName Id;

	/** Stage for StartQuest (optional) and SetQuestStage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Consequence",
		meta=(EditCondition="Type==EDCConsequenceType::StartQuest||Type==EDCConsequenceType::SetQuestStage", EditConditionHides))
	FName Stage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Consequence",
		meta=(ClampMin="1", EditCondition="Type==EDCConsequenceType::GiveItem||Type==EDCConsequenceType::RemoveItem", EditConditionHides))
	int32 Quantity = 1;

	/** Optional direct item reference for GiveItem / RemoveItem. Id is used when this is unset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Consequence",
		meta=(EditCondition="Type==EDCConsequenceType::GiveItem||Type==EDCConsequenceType::RemoveItem", EditConditionHides))
	TSoftObjectPtr<UDCItemDefinition> Item;
};
