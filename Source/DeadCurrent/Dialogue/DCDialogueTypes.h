#pragma once

#include "CoreMinimal.h"
#include "DCDialogueTypes.generated.h"

/**
 *  One player reply. NextNodeId empty ends the conversation.
 *  Conditions and consequences can hang off this later without changing callers.
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
