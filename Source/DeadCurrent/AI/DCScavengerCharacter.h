#pragma once

#include "CoreMinimal.h"
#include "Combat/DCDamageable.h"
#include "GameFramework/Character.h"
#include "Interaction/DCInteractable.h"
#include "Save/DCPersistent.h"
#include "DCScavengerCharacter.generated.h"

class UDCHealthComponent;
class UDCInventoryComponent;
class UDCPersistentIdComponent;

/**
 *  Hostile scavenger. Combat brain lives on ADCScavengerController.
 *  After death the body is a loot container using the same inventory component.
 */
UCLASS()
class DEADCURRENT_API ADCScavengerCharacter : public ACharacter, public IDCDamageable, public IDCInteractable, public IDCPersistent
{
	GENERATED_BODY()

public:
	ADCScavengerCharacter();

	virtual void ApplyDamage_Implementation(const FDCDamageInfo& Damage) override;

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual FDCInteractionPrompt GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	virtual FGameplayTag GetInteractionType_Implementation() const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	virtual FName GetPersistentId_Implementation() const override;
	virtual void CapturePersistentState_Implementation(FDCPersistentActorState& OutState) const override;
	virtual void ApplyPersistentState_Implementation(const FDCPersistentActorState& State) override;

	/** Swing at Target if in range and off cooldown. Returns true if damage was applied. */
	UFUNCTION(BlueprintCallable, Category="Combat")
	bool TryMelee(AActor* Target);

	UDCHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UDCInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	const TArray<FVector>& GetPatrolPoints() const { return PatrolPoints; }

	UFUNCTION(BlueprintPure, Category="Combat")
	float GetMeleeRange() const { return MeleeRange; }

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UDCHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UDCInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UDCPersistentIdComponent> PersistentIdComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI")
	TArray<FVector> PatrolPoints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat", meta=(ClampMin="0", Units="cm"))
	float MeleeRange = 170.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat", meta=(ClampMin="0"))
	float MeleeDamage = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat", meta=(ClampMin="0", Units="s"))
	float MeleeCooldown = 1.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Target")
	FText DisplayName;

private:

	UFUNCTION()
	void HandleDied(UDCHealthComponent* Health, const FDCDamageInfo& Damage);

	void Die();

	void GrantStartingLoot();

	double NextMeleeTime = 0.0;

	bool bStartingLootGranted = false;
};
