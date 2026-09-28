#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Dialogue/DCDialogueTypes.h"
#include "DCDialogueAsset.generated.h"

/**
 *  One conversation graph. Create under /Game/Dialogue as DA_Dialogue_<Name>.
 *  Entries pick the opening node from quest/world conditions. Choices can hide or fire consequences.
 */
UCLASS(BlueprintType, Const)
class DEADCURRENT_API UDCDialogueAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dialogue")
	FName DialogueId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dialogue")
	FName EntryNodeId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dialogue")
	TArray<FDCDialogueNode> Nodes;

	/** First matching entry starts the conversation. Empty falls back to EntryNodeId. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dialogue")
	TArray<FDCDialogueEntry> Entries;

	const FDCDialogueNode* FindNode(FName NodeId) const;

	/** First entry whose conditions pass for Context, otherwise EntryNodeId. */
	FName ResolveEntry(const struct FDCRuleContext& Context) const;

#if WITH_EDITOR
	/** Graph checks: entry and next nodes exist, node ids are unique, every node can be left. */
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
