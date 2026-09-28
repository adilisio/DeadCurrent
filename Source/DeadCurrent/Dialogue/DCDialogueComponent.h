#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dialogue/DCDialogueTypes.h"
#include "DCDialogueComponent.generated.h"

class UDCDialogueAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDCDialogueEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDCDialogueNodeEvent, FName, NodeId);

/**
 *  Runs a dialogue asset on the player. NPCs call StartDialogue; the HUD and digit keys drive choices.
 */
UCLASS(ClassGroup=(DeadCurrent), meta=(BlueprintSpawnableComponent))
class DEADCURRENT_API UDCDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDCDialogueComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Begin a conversation. Returns false if already talking or the asset has no entry node. */
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	bool StartDialogue(const UDCDialogueAsset* Asset, AActor* Participant);

	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void EndDialogue();

	/** Select a 0-based choice on the current node. Returns false if out of range or not talking. */
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	bool SelectChoice(int32 ChoiceIndex);

	UFUNCTION(BlueprintPure, Category="Dialogue")
	bool IsInDialogue() const { return ActiveAsset != nullptr; }

	UFUNCTION(BlueprintPure, Category="Dialogue")
	const UDCDialogueAsset* GetActiveAsset() const { return ActiveAsset; }

	UFUNCTION(BlueprintPure, Category="Dialogue")
	AActor* GetParticipant() const { return Participant.Get(); }

	UFUNCTION(BlueprintPure, Category="Dialogue")
	FName GetCurrentNodeId() const { return CurrentNodeId; }

	const FDCDialogueNode* GetCurrentNode() const;

	/** 0-based indices into the current node's Choices that pass conditions. */
	TArray<int32> GetVisibleChoiceIndices() const;

	UPROPERTY(BlueprintAssignable, Category="Dialogue")
	FDCDialogueEvent OnDialogueStarted;

	UPROPERTY(BlueprintAssignable, Category="Dialogue")
	FDCDialogueEvent OnDialogueEnded;

	UPROPERTY(BlueprintAssignable, Category="Dialogue")
	FDCDialogueNodeEvent OnNodeChanged;

private:

	class UDCQuestComponent* GetQuestComponent() const;

	bool AdvanceTo(FName NodeId);

	UPROPERTY()
	TObjectPtr<const UDCDialogueAsset> ActiveAsset;

	TWeakObjectPtr<AActor> Participant;

	FName CurrentNodeId;
};
