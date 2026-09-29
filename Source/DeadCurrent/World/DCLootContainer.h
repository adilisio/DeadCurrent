#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/DCInteractable.h"
#include "Save/DCPersistent.h"
#include "DCLootContainer.generated.h"

class UDCInventoryComponent;
class UDCPersistentIdComponent;
class UStaticMeshComponent;

/**
 *  A world container the player loots: crate, locker, toolbox, cache. One class for all of them:
 *  a placed container differs only by mesh, DisplayName, persistent id and starting contents (the
 *  inventory component's authored Stacks). E takes the next stack, like a corpse; the prompt names
 *  it and the message lists what is left.
 *
 *  Contents persist through the save system's world-inventory capture like any persistent actor with
 *  an inventory, so partial and full looting survive save/load with no container-specific save code.
 *  A save that predates the container leaves its authored contents alone.
 */
UCLASS()
class DEADCURRENT_API ADCLootContainer : public AActor, public IDCInteractable, public IDCPersistent
{
	GENERATED_BODY()

public:
	ADCLootContainer();

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual FDCInteractionPrompt GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	virtual FGameplayTag GetInteractionType_Implementation() const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	virtual FName GetPersistentId_Implementation() const override;
	virtual void CapturePersistentState_Implementation(FDCPersistentActorState& OutState) const override;
	virtual void ApplyPersistentState_Implementation(const FDCPersistentActorState& State) override;

	/** Moves the next stack into Taker's inventory. Returns how many items moved (0 when empty). */
	UFUNCTION(BlueprintCallable, Category="Container")
	int32 TakeNext(AActor* Taker);

	UFUNCTION(BlueprintPure, Category="Container")
	bool IsEmpty() const;

	UFUNCTION(BlueprintPure, Category="Container")
	FText GetDisplayName() const { return DisplayName; }

	UDCInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** Author the starting contents on this component's Stacks. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UDCInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UDCPersistentIdComponent> PersistentIdComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Container")
	FText DisplayName;

private:

	/** "9mm Rounds (12)" */
	static FText DescribeStack(const struct FDCItemStack& Stack);
};
