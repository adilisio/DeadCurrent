#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/DCGameplayTypes.h"
#include "Interaction/DCInteractable.h"
#include "DCInspectableActor.generated.h"

class UStaticMeshComponent;

/**
 *  One conditional reading of an inspectable. The first variant whose conditions pass for the
 *  interactor is shown and its consequences run (for example, set a clue flag the first time).
 */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCInspectVariant
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspect")
	TArray<FDCGameplayCondition> Conditions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspect", meta=(MultiLine="true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspect")
	TArray<FDCGameplayConsequence> Consequences;
};

/**
 *  A world prop that shows a short description when inspected. Variants let the text react to
 *  quests and world state, and let inspecting change them (clues).
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

	/** Checked in order before Description; the first whose conditions pass is shown. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspect")
	TArray<FDCInspectVariant> Variants;

	/** Seconds the description stays on screen */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspect", meta=(ClampMin="0", Units="s"))
	float DescriptionDuration = 5.0f;
};
