#pragma once

#include "CoreMinimal.h"
#include "Combat/DCDamageable.h"
#include "GameFramework/Actor.h"
#include "DCShootableTarget.generated.h"

class UDCHealthComponent;
class UStaticMeshComponent;

/**
 *  A range plate with health. Hits go through UDCHealthComponent; it falls over when it dies.
 */
UCLASS()
class DEADCURRENT_API ADCShootableTarget : public AActor, public IDCDamageable
{
	GENERATED_BODY()

public:
	ADCShootableTarget();

	virtual void ApplyDamage_Implementation(const FDCDamageInfo& Damage) override;

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UDCHealthComponent> HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Target")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Target", meta=(ClampMin="0", Units="s"))
	float FlashDuration = 0.12f;

private:

	UFUNCTION()
	void HandleDied(UDCHealthComponent* Health, const FDCDamageInfo& Damage);

	void EndFlash();

	FVector BaseScale = FVector::OneVector;

	FTimerHandle FlashTimer;
};
