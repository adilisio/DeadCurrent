#pragma once

#include "CoreMinimal.h"
#include "Core/DCGameplayTypes.h"
#include "DCDialogueTypes.generated.h"

/**
 *  One player reply. NextNodeId empty ends the conversation.
 */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCDialogueChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue")
	FText Text;

	/** Node to show next. None ends the conversation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue")
	FName NextNodeId;

	/** Hidden when any condition fails. Empty means always shown. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue")
	TArray<FDCGameplayCondition> Conditions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue")
	TArray<FDCGameplayConsequence> Consequences;
};

/**
 *  Optional conversation start. First entry whose conditions pass wins; otherwise EntryNodeId.
 */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCDialogueEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue")
	FName NodeId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue")
	TArray<FDCGameplayCondition> Conditions;
};

/**
 *  One spoken line and the replies available from it.
 */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCDialogueNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue")
	FName NodeId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue")
	FText Speaker;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue", meta=(MultiLine="true"))
	FText Line;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue")
	TArray<FDCDialogueChoice> Choices;
};
