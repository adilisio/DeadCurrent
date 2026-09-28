#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/DCInteractable.h"
#include "Save/DCPersistent.h"
#include "DCItemPickup.generated.h"

class UDCItemDefinition;
class UDCPersistentIdComponent;
class UStaticMeshComponent;

/**
 *  An item lying in the world. Its name and mesh come from the item definition.
 */
UCLASS()
class DEADCURRENT_API ADCItemPickup : public AActor, public IDCInteractable, public IDCPersistent
{
	GENERATED_BODY()

public:
	ADCItemPickup();

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual FDCInteractionPrompt GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	virtual FGameplayTag GetInteractionType_Implementation() const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	virtual FName GetPersistentId_Implementation() const override;
	virtual void CapturePersistentState_Implementation(FDCPersistentActorState& OutState) const override;
	virtual void ApplyPersistentState_Implementation(const FDCPersistentActorState& State) override;

	UFUNCTION(BlueprintPure, Category="Pickup")
	const UDCItemDefinition* GetItem() const { return Item; }

	UFUNCTION(BlueprintPure, Category="Pickup")
	int32 GetQuantity() const { return Quantity; }

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UDCPersistentIdComponent> PersistentIdComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup")
	TObjectPtr<const UDCItemDefinition> Item;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup", meta=(ClampMin="1"))
	int32 Quantity = 1;

private:

	void ApplyItemMesh();
};
