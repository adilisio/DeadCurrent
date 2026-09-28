#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Save/DCPersistentTypes.h"
#include "DCQuestComponent.generated.h"

class UDCInventoryComponent;
class UDCQuestDefinition;
struct FDCQuestStage;
struct FDCRuleContext;

UENUM(BlueprintType)
enum class EDCQuestStatus : uint8
{
	NotStarted,
	Active,
	Complete
};

/** One started quest in the log. */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCQuestProgress
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Quest")
	FName QuestId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Quest")
	FName StageId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDCQuestUpdated, FName, QuestId, FName, StageId);

/**
 *  The player's quest log: which quests are started and the current stage of each.
 *  Stage rules (objectives, transitions, outcomes) live on UDCQuestDefinition assets.
 *  Transitions are re-checked whenever the owner's inventory, the world state, or a quest stage changes.
 */
UCLASS(ClassGroup=(DeadCurrent), meta=(BlueprintSpawnableComponent))
class DEADCURRENT_API UDCQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Start a quest at StageId, or at the definition's start stage when StageId is None. False if already started. */
	UFUNCTION(BlueprintCallable, Category="Quest")
	bool StartQuest(FName QuestId, FName StageId = NAME_None);

	/** Move a started quest to StageId, apply that stage's OnEnter consequences, then re-check transitions. */
	UFUNCTION(BlueprintCallable, Category="Quest")
	bool SetStage(FName QuestId, FName StageId);

	UFUNCTION(BlueprintPure, Category="Quest")
	FName GetStage(FName QuestId) const;

	UFUNCTION(BlueprintPure, Category="Quest")
	EDCQuestStatus GetQuestStatus(FName QuestId) const;

	UFUNCTION(BlueprintPure, Category="Quest")
	bool HasQuest(FName QuestId) const { return FindProgress(QuestId) != nullptr; }

	UFUNCTION(BlueprintPure, Category="Quest")
	bool IsComplete(FName QuestId) const { return GetQuestStatus(QuestId) == EDCQuestStatus::Complete; }

	UFUNCTION(BlueprintPure, Category="Quest")
	bool IsQuestActive(FName QuestId) const { return GetQuestStatus(QuestId) == EDCQuestStatus::Active; }

	/** Objective of the first active quest with one, for the HUD. */
	UFUNCTION(BlueprintPure, Category="Quest")
	FText GetObjectiveText() const;

	/** The quest the HUD tracks: the first active quest that has objective text. */
	UFUNCTION(BlueprintPure, Category="Quest")
	FName GetTrackedQuestId() const;

	/** Objective (or outcome summary, once complete) of one quest's current stage. */
	UFUNCTION(BlueprintPure, Category="Quest")
	FText GetStageText(FName QuestId) const;

	/** Every started quest, in the order they were started. */
	const TArray<FDCQuestProgress>& GetQuestLog() const { return Quests; }

	/** Re-check transitions of every active quest. Runs automatically; callable for scripted events and tests. */
	UFUNCTION(BlueprintCallable, Category="Quest")
	void EvaluateQuests();

	void CaptureState(TArray<FDCSavedQuestState>& OutQuests) const;

	/** Replaces the log from a save. Does not apply OnEnter consequences or re-check transitions. */
	void ReplaceFromSaved(const TArray<FDCSavedQuestState>& SavedQuests);

	/** Fires after a quest starts or changes stage. */
	UPROPERTY(BlueprintAssignable, Category="Quest")
	FDCQuestUpdated OnQuestUpdated;

protected:

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	FDCQuestProgress* FindProgress(FName QuestId);

	const FDCQuestProgress* FindProgress(FName QuestId) const;

	const FDCQuestStage* FindCurrentStage(const FDCQuestProgress& Progress) const;

	FDCRuleContext MakeRuleContext();

	void EnterStage(FName QuestId, FName StageId);

	UFUNCTION()
	void HandleInventoryChanged(UDCInventoryComponent* Inventory);

	void HandleWorldStateChanged();

	UPROPERTY()
	TArray<FDCQuestProgress> Quests;

	FDelegateHandle WorldStateHandle;

	bool bEvaluating = false;

	bool bEvaluateAgain = false;
};
