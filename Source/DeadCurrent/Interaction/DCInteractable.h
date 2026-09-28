#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "DCInteractable.generated.h"

/** What the interaction prompt shows, e.g. Action "Open", Target "Boathouse Door". */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCInteractionPrompt
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FText Action;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FText TargetName;
};

UINTERFACE(BlueprintType)
class DEADCURRENT_API UDCInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 *  Anything the player can look at and interact with: pickups, containers, corpses, NPCs, doors, props.
 *  Implement in C++ or Blueprint; call through the Execute_ wrappers.
 */
class DEADCURRENT_API IDCInteractable
{
	GENERATED_BODY()

public:

	/** Whether Interactor can use this right now. Unavailable interactables show no prompt. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	bool CanInteract(AActor* Interactor) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	FDCInteractionPrompt GetInteractionPrompt(AActor* Interactor) const;

	/** Interaction.* tag describing the kind of interaction */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	FGameplayTag GetInteractionType() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void Interact(AActor* Interactor);
};
