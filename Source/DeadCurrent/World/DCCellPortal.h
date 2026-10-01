#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/DCGameplayTypes.h"
#include "Interaction/DCInteractable.h"
#include "DCCellPortal.generated.h"

class UStaticMeshComponent;

/** What a use of a cell portal did. */
UENUM(BlueprintType)
enum class EDCPortalUse : uint8
{
	/** Nothing passed: LockedText was shown and nothing changed. */
	Locked,
	/** A transition is already running (or the portal has no destination): the use did nothing. */
	Ignored,
	/** A variant passed and the transition started (a timed portal) or completed (an instant one). */
	Passed
};

/**
 *  One way a cell portal can be used. The first variant whose conditions pass for the interactor is used; its verb is
 *  the prompt and its consequences run when the player passes through.
 */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCPortalVariant
{
	GENERATED_BODY()

	/** Readable name for logs and tests ("key", "forced", "open"). Not saved. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Portal")
	FName VariantId;

	/** All must pass. An empty list always passes, so an unconditioned last variant is the portal's open state. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Portal")
	TArray<FDCGameplayCondition> Conditions;

	/** Prompt verb while this variant applies ("Unlock", "Pry the jam", "Go down"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Portal")
	FText Verb = NSLOCTEXT("DCCellPortal", "DefaultVerb", "Go");

	/** Applied once per use, while the screen is dark, before the player is moved. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Portal")
	TArray<FDCGameplayConsequence> Consequences;
};

/**
 *  Cell portal (Phase 6): moves the player between two places in the same map, such as an exterior and an interior cell
 *  that is built somewhere else in the level (a vault under a lighthouse, a loft over a store, the top of a stair).
 *
 *  Using it fades the screen out, applies the chosen variant's consequences, moves the player to Destination, signals a
 *  scene cut (UDCWorldStateSubsystem::NotifySceneCut, on which conditional presence snaps), and fades back in. With
 *  every fade duration at zero it does all of that at once, synchronously.
 *
 *  Variants are ordered and the first whose conditions pass wins, like inspect variants. When none passes the portal
 *  is locked: it shows LockedText and nothing else happens. That is condition-driven locked access; there is no
 *  separate locked-door class. Both directions of a doorway are two portals.
 *
 *  It saves nothing. A portal's lasting effect is whatever flags its consequences set, which are saved already, and
 *  the player's position, which every save records. Do not use it for anything that must be remembered on its own.
 */
UCLASS()
class DEADCURRENT_API ADCCellPortal : public AActor, public IDCInteractable
{
	GENERATED_BODY()

public:
	ADCCellPortal();

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual FDCInteractionPrompt GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	virtual FGameplayTag GetInteractionType_Implementation() const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintPure, Category="Portal")
	FText GetDisplayName() const { return DisplayName; }

	/** Interact calls this. Returns what the use did, so tests and tools can tell locked from passed without a HUD. */
	UFUNCTION(BlueprintCallable, Category="Portal")
	EDCPortalUse TryUse(AActor* Interactor);

	/** Index into Variants of the first that passes for Interactor, or INDEX_NONE when the portal is locked to them. */
	int32 FindVariant(AActor* Interactor) const;

	/** True from a timed use until the fade back in has finished. Uses are ignored meanwhile. */
	UFUNCTION(BlueprintPure, Category="Portal")
	bool IsTransitioning() const { return bTransitioning; }

	/** All three fade durations are zero: a use completes synchronously. */
	bool IsInstant() const;

	AActor* GetDestination() const { return Destination; }

protected:

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** What the interaction trace hits: the door, hatch, or stair this portal stands for. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Portal")
	FText DisplayName;

	/** Checked in order; the first whose conditions pass for the interactor is used. None passing: locked. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Portal")
	TArray<FDCPortalVariant> Variants;

	/** Prompt verb while locked. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Portal")
	FText LockedVerb;

	/** Shown when the portal is used while locked ("The hatch is locked."). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Portal", meta=(MultiLine="true"))
	FText LockedText;

	/** Where the player arrives (an actor in the same level, usually an empty marker): its location, and its yaw if set. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Portal")
	TObjectPtr<AActor> Destination;

	/** Turn the player to face the destination's yaw on arrival (pitch is levelled). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Portal")
	bool bUseDestinationYaw = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Portal", meta=(ClampMin="0", Units="s"))
	float FadeOutSeconds = 0.35f;

	/** Time the screen stays black after the move. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Portal", meta=(ClampMin="0", Units="s"))
	float HoldSeconds = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Portal", meta=(ClampMin="0", Units="s"))
	float FadeInSeconds = 0.35f;

	/** Optional line shown while the screen is dark ("You climb the stair."). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Portal", meta=(MultiLine="true"))
	FText CardText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Portal", meta=(ClampMin="0", Units="s"))
	float CardSeconds = 2.5f;

private:

	/** At black: consequences, move, scene cut, card. */
	void Cut(AActor* Interactor, int32 VariantIndex);

	void BeginFadeIn();

	void FinishTransition();

	/** Movement and look input off (true) or back on (false), and the screen fade, for a player-controlled interactor. */
	void SetPlayerLocked(AActor* Interactor, bool bLocked) const;

	void Fade(AActor* Interactor, float From, float To, float Seconds, bool bHoldWhenFinished) const;

	bool bTransitioning = false;

	TWeakObjectPtr<AActor> PendingInteractor;

	int32 PendingVariant = INDEX_NONE;

	FTimerHandle StepTimer;
};
