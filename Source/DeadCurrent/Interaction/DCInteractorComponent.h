#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interaction/DCInteractable.h"
#include "DCInteractorComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDCInteractionFocusChanged, AActor*, NewFocus, AActor*, OldFocus);

/**
 *  Finds the interactable the owning player is looking at and interacts with it on request.
 *  Requires the owner to be a pawn controlled by a player controller.
 */
UCLASS(ClassGroup=(DeadCurrent), meta=(BlueprintSpawnableComponent))
class DEADCURRENT_API UDCInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDCInteractorComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Interacts with the focused actor. Returns false if there is nothing usable in focus. */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	bool TryInteract();

	/** The interactable currently in focus and usable, or null */
	UFUNCTION(BlueprintPure, Category="Interaction")
	AActor* GetFocusedActor() const { return FocusedActor.Get(); }

	/** Prompt for the focused actor. Returns false if nothing is in focus. */
	UFUNCTION(BlueprintPure, Category="Interaction")
	bool GetFocusedPrompt(FDCInteractionPrompt& OutPrompt) const;

	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FDCInteractionFocusChanged OnFocusChanged;

protected:

	/** Maximum distance from the view point to the interactable surface */
	UPROPERTY(EditAnywhere, Category="Interaction", meta=(ClampMin="0", Units="cm"))
	float InteractionRange = 250.0f;

	/** Radius of the focus sweep, so small objects do not need pixel-perfect aim */
	UPROPERTY(EditAnywhere, Category="Interaction", meta=(ClampMin="0", Units="cm"))
	float TraceRadius = 8.0f;

	UPROPERTY(EditAnywhere, Category="Interaction")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

private:

	AActor* FindInteractable() const;

	void SetFocus(AActor* NewFocus);

	TWeakObjectPtr<AActor> FocusedActor;
};
