#pragma once

#include "CoreMinimal.h"
#include "Combat/DCDamageable.h"
#include "GameFramework/Actor.h"
#include "DCShootableTarget.generated.h"

class UStaticMeshComponent;

/**
 *  A range plate that counts hits. Used in the test gym until enemies exist.
 */
UCLASS()
class DEADCURRENT_API ADCShootableTarget : public AActor, public IDCDamageable
{
	GENERATED_BODY()

public:
	ADCShootableTarget();

	virtual void ApplyDamage_Implementation(const FDCDamageInfo& Damage) override;

	UFUNCTION(BlueprintPure, Category="Target")
	int32 GetHitCount() const { return HitCount; }

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Target")
	FText DisplayName;

	/** Seconds the hit flash lasts */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Target", meta=(ClampMin="0", Units="s"))
	float FlashDuration = 0.12f;

private:

	void EndFlash();

	FVector BaseScale = FVector::OneVector;

	int32 HitCount = 0;

	FTimerHandle FlashTimer;
};
