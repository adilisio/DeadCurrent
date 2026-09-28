#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/DCGameplayTypes.h"
#include "Save/DCPersistentTypes.h"
#include "DCQuestComponent.generated.h"

class UDCQuestDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDCQuestUpdated, FName, QuestId, FName, StageId);

/**
 *  Player quest log and world flags. Dialogue conditions and consequences run through here.
 */
UCLASS(ClassGroup=(DeadCurrent), meta=(BlueprintSpawnableComponent))
class DEADCURRENT_API UDCQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="Quest")
	bool StartQuest(FName QuestId, FName StageId);

	UFUNCTION(BlueprintCallable, Category="Quest")
	bool SetStage(FName QuestId, FName StageId);

	UFUNCTION(BlueprintCallable, Category="Quest")
	bool CompleteQuest(FName QuestId);

	UFUNCTION(BlueprintPure, Category="Quest")
	FName GetStage(FName QuestId) const;

	UFUNCTION(BlueprintPure, Category="Quest")
	bool HasQuest(FName QuestId) const { return Stages.Contains(QuestId); }

	UFUNCTION(BlueprintPure, Category="Quest")
	bool IsComplete(FName QuestId) const;

	UFUNCTION(BlueprintPure, Category="Quest")
	bool IsQuestActive(FName QuestId) const { return HasQuest(QuestId) && !IsComplete(QuestId); }

	UFUNCTION(BlueprintPure, Category="Quest")
	FText GetObjectiveText() const;

	UFUNCTION(BlueprintCallable, Category="Quest")
	void SetFlag(FName Flag);

	UFUNCTION(BlueprintPure, Category="Quest")
	bool HasFlag(FName Flag) const { return Flag.IsNone() ? false : Flags.Contains(Flag); }

	/** Called when a scavenger dies. Advances stages that opted into hostile-death. */
	UFUNCTION(BlueprintCallable, Category="Quest")
	void NotifyHostileDied();

	bool Meets(const FDCGameplayCondition& Condition) const;

	bool MeetsAll(const TArray<FDCGameplayCondition>& Conditions) const;

	void Apply(const FDCGameplayConsequence& Consequence);

	void ApplyAll(const TArray<FDCGameplayConsequence>& Consequences);

	void CaptureState(TArray<FDCSavedQuestState>& OutQuests, TArray<FName>& OutFlags) const;

	void ReplaceFromSaved(const TArray<FDCSavedQuestState>& SavedQuests, const TArray<FName>& SavedFlags);

	UPROPERTY(BlueprintAssignable, Category="Quest")
	FDCQuestUpdated OnQuestUpdated;

private:

	FName CompletedStageId(FName QuestId) const;

	const UDCQuestDefinition* FindDefinition(FName QuestId) const;

	bool IsHostileDead() const;

	bool HasItem(FName ItemId, int32 Quantity) const;

	void GiveOrRemoveItem(FName ItemId, const TSoftObjectPtr<UDCItemDefinition>& Item, int32 Quantity, bool bGive);

	void BroadcastIfLive(FName QuestId, FName StageId);

	UPROPERTY()
	TMap<FName, FName> Stages;

	UPROPERTY()
	TArray<FName> Flags;
};
