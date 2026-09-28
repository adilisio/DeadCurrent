#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/DCInteractable.h"
#include "DCInspectableActor.generated.h"

class UStaticMeshComponent;

/**
 *  A world prop that shows a short description when inspected.
 */
UCLASS()
class DEADCURRENT_API ADCInspectableActor : public AActor, public IDCInteractable
{
	GENERATED_BODY()

public:
	ADCInspectableActor();

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual FDCInteractionPrompt GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	virtual FGameplayTag GetInteractionType_Implementation() const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspect")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspect", meta=(MultiLine="true"))
	FText Description;

	/** Seconds the description stays on screen */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspect", meta=(ClampMin="0", Units="s"))
	float DescriptionDuration = 5.0f;
};
