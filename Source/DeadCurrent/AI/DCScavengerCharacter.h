#pragma once

#include "CoreMinimal.h"
#include "Combat/DCDamageable.h"
#include "GameFramework/Character.h"
#include "DCScavengerCharacter.generated.h"

class UDCHealthComponent;
class UDCInventoryComponent;

/**
 *  Hostile scavenger. Combat brain lives on ADCScavengerController.
 */
UCLASS()
class DEADCURRENT_API ADCScavengerCharacter : public ACharacter, public IDCDamageable
{
	GENERATED_BODY()

public:
	ADCScavengerCharacter();

	virtual void ApplyDamage_Implementation(const FDCDamageInfo& Damage) override;

	/** Swing at Target if in range and off cooldown. Returns true if damage was applied. */
	UFUNCTION(BlueprintCallable, Category="Combat")
	bool TryMelee(AActor* Target);

	UDCHealthComponent* GetHealthComponent() const { return HealthComponent; }

	const TArray<FVector>& GetPatrolPoints() const { return PatrolPoints; }

	UFUNCTION(BlueprintPure, Category="Combat")
	float GetMeleeRange() const { return MeleeRange; }

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UDCHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UDCInventoryComponent> InventoryComponent;

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

	double NextMeleeTime = 0.0;
};
