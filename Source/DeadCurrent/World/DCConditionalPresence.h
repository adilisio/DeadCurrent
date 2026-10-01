#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/DCGameplayTypes.h"
#include "DCConditionalPresence.generated.h"

/**
 *  One authored state of a conditional presence rule: where its targets are, and whether they are there at all.
 */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCPresenceState
{
	GENERATED_BODY()

	/** Readable name for logs, tests, and review ("coil", "combat"). Not saved. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Presence")
	FName StateId;

	/** All must pass. An empty list always passes, so an unconditioned last state is an explicit default. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Presence")
	TArray<FDCGameplayCondition> Conditions;

	/** False hides the targets and turns their collision off. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Presence")
	bool bPresent = true;

	/** Move the targets to Placement. Otherwise they stay at their authored transforms. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Presence")
	bool bMove = false;

	/** World transform the rule actor (the pivot) takes in this state. Each target keeps its authored offset from it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Presence", meta=(EditCondition="bMove"))
	FTransform Placement;
};

/**
 *  Conditional presence: "these actors are here, somewhere else, or not here at all, when these conditions pass."
 *
 *  A rule actor, placed where its targets are authored (it is their pivot). States are checked in order and the
 *  first whose conditions pass wins; when none pass, the targets are present at their authored transforms.
 *  Conditions are the shared FDCGameplayCondition language, evaluated against the local player's pawn when there
 *  is one (so item, quest, and build checks work) and against this actor otherwise, like ADCConditionalAudio.
 *
 *  It saves nothing and sets nothing. Its result is recomputed from existing state, so a save from before a rule
 *  existed shows whatever that save's flags imply. It snaps at level start and when the world state reports a
 *  restore (a load, a review setup) or a scene cut (a cell portal moving the player while the screen is dark).
 *  Any other change waits while the player is near the targets' current or new place, or while a target is on
 *  screen, so a character does not vanish mid-conversation or appear at the player's feet.
 *
 *  Use it for presentation and placement that follow state already stored somewhere. Anything that must be
 *  remembered on its own (an object the player moved, a door left open) belongs in IDCPersistent instead.
 */
UCLASS()
class DEADCURRENT_API ADCConditionalPresence : public AActor
{
	GENERATED_BODY()

public:
	ADCConditionalPresence();

	virtual void Tick(float DeltaSeconds) override;

	/** Index into States of the state that should apply now, or INDEX_NONE for the authored default. */
	UFUNCTION(BlueprintPure, Category="Presence")
	int32 FindPassingState() const;

	/** The state the targets are in (NAME_None for the authored default). */
	UFUNCTION(BlueprintPure, Category="Presence")
	FName GetActiveStateId() const;

	/** A different state passes but is waiting until the player is away and not looking. */
	UFUNCTION(BlueprintPure, Category="Presence")
	bool HasPendingChange() const { return bHasPending; }

	/** Re-check now; applies at once unless the change is deferred. Tick calls this every CheckInterval. */
	void Evaluate();

	/** Re-check now and apply at once, ignoring deferral (level start, save restore, review setup). */
	void Snap();

	/** Tests and tools: judge "observed" from this point instead of the player pawn. Unset to go back to the pawn. */
	void SetObserverOverride(TOptional<FVector> InObserver) { ObserverOverride = InObserver; }

	const TArray<TObjectPtr<AActor>>& GetTargets() const { return Targets; }

protected:

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Pivot;

	/** Actors in the same level whose presence and placement this rule owns. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Presence")
	TArray<TObjectPtr<AActor>> Targets;

	/** Checked in order; first passing state wins. None passing: authored transforms, present. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Presence")
	TArray<FDCPresenceState> States;

	/** Hold a change while the player is near or a target is on screen. Level start and restores always snap. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Presence")
	bool bDeferWhileObserved = true;

	/** Player within this distance of the targets' current or next pivot counts as observing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Presence", meta=(ClampMin="0", Units="cm"))
	float ObservedDistance = 1500.0f;

	/** Seconds between checks. Flag changes are also picked up at once through the world-state signal. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Presence", meta=(ClampMin="0.05", Units="s"))
	float CheckInterval = 0.5f;

private:

	struct FAuthored
	{
		TWeakObjectPtr<AActor> Actor;
		FTransform Offset;               // target relative to the authored pivot
		bool bCollisionEnabled = true;   // as authored, restored when shown again
	};

	void Apply(int32 StateIndex);

	bool IsObserved(int32 NextStateIndex) const;

	FTransform PivotFor(int32 StateIndex) const;

	void HandleWorldChanged();

	TArray<FAuthored> Authored;

	FTransform AuthoredPivot;

	int32 ActiveState = INDEX_NONE;

	bool bApplied = false;

	bool bHasPending = false;

	float TimeToCheck = 0.0f;

	TOptional<FVector> ObserverOverride;

	FDelegateHandle ChangedHandle;

	FDelegateHandle RestoredHandle;

	FDelegateHandle SceneCutHandle;
};
