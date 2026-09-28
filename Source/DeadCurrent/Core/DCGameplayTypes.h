#pragma once

#include "CoreMinimal.h"
#include "DCGameplayTypes.generated.h"

class UDCItemDefinition;

UENUM(BlueprintType)
enum class EDCConditionType : uint8
{
	None UMETA(Hidden),
	QuestStage,
	QuestNotStarted,
	QuestActive,
	QuestComplete,
	HostileDead,
	HasItem,
	WorldFlag
};

UENUM(BlueprintType)
enum class EDCConsequenceType : uint8
{
	None UMETA(Hidden),
	StartQuest,
	SetQuestStage,
	CompleteQuest,
	GiveItem,
	RemoveItem,
	SetWorldFlag
};

/**
 *  One check used by dialogue and quests. All conditions on a choice or entry are ANDed.
 */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCGameplayCondition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Condition")
	EDCConditionType Type = EDCConditionType::None;

	/** Quest id, item id, or world flag, depending on Type. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Condition")
	FName Id;

	/** Quest stage when Type is QuestStage. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Condition")
	FName Stage;

	/** Minimum quantity when Type is HasItem. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Condition", meta=(ClampMin="1"))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Condition")
	bool bNegate = false;
};

/**
 *  One result of a dialogue choice (or later a quest event).
 */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCGameplayConsequence
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consequence")
	EDCConsequenceType Type = EDCConsequenceType::None;

	/** Quest id, item id, or world flag, depending on Type. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consequence")
	FName Id;

	/** Stage to set when Type is StartQuest or SetQuestStage. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consequence")
	FName Stage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consequence", meta=(ClampMin="1"))
	int32 Quantity = 1;

	/** Optional direct item for GiveItem / RemoveItem. Id is used if this is unset. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consequence")
	TSoftObjectPtr<UDCItemDefinition> Item;
};
