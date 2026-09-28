#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Dialogue/DCDialogueTypes.h"
#include "DCDialogueAsset.generated.h"

/**
 *  One conversation graph. Create under /Game/Dialogue as DA_Dialogue_<Name>.
 *  Conditions / checks / consequences can be added to choices later; the runtime already walks by NodeId.
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

	const FDCDialogueNode* FindNode(FName NodeId) const;
};
