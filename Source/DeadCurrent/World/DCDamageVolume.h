#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DCDamageVolume.generated.h"

class UStaticMeshComponent;

/**
 *  An area that damages overlapping pawns each second. Used for environmental hazards.
 */
UCLASS()
class DEADCURRENT_API ADCDamageVolume : public AActor
{
	GENERATED_BODY()

public:
	ADCDamageVolume();

	virtual void Tick(float DeltaSeconds) override;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage", meta=(ClampMin="0"))
	float DamagePerSecond = 20.0f;

private:

	TSet<TWeakObjectPtr<AActor>> WarnedActors;
};
