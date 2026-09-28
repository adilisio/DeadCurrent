#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/DCInteractable.h"
#include "Save/DCPersistent.h"
#include "DCDoor.generated.h"

class UStaticMeshComponent;
class UDCPersistentIdComponent;

/**
 *  A hinged door that swings open away from whoever uses it.
 *  The actor origin is the hinge; the door leaf should extend along the actor's +Y axis.
 */
UCLASS()
class DEADCURRENT_API ADCDoor : public AActor, public IDCInteractable, public IDCPersistent
{
	GENERATED_BODY()

public:
	ADCDoor();

	virtual void Tick(float DeltaSeconds) override;

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual FDCInteractionPrompt GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	virtual FGameplayTag GetInteractionType_Implementation() const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	virtual FName GetPersistentId_Implementation() const override;
	virtual void CapturePersistentState_Implementation(FDCPersistentActorState& OutState) const override;
	virtual void ApplyPersistentState_Implementation(const FDCPersistentActorState& State) override;

	UFUNCTION(BlueprintPure, Category="Door")
	bool IsOpen() const { return bIsOpen; }

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Hinge;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UDCPersistentIdComponent> PersistentIdComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Door")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Door", meta=(ClampMin="0", ClampMax="180", Units="deg"))
	float OpenAngle = 95.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Door", meta=(ClampMin="1", Units="deg/s"))
	float SwingSpeed = 180.0f;

private:

	bool bIsOpen = false;

	float ClosedYaw = 0.0f;

	float TargetYaw = 0.0f;

	float CurrentYaw = 0.0f;
};
