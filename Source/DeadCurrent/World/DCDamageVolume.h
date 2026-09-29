#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/DCGameplayTypes.h"
#include "DCDamageVolume.generated.h"

class UStaticMeshComponent;

/**
 *  An area that damages overlapping pawns each second. Used for environmental hazards.
 *  ActiveConditions can switch it off from world state (a live-water hazard that stops once the
 *  battery is disconnected); while inactive it deals no damage and hides its mesh.
 */
UCLASS()
class DEADCURRENT_API ADCDamageVolume : public AActor
{
	GENERATED_BODY()

public:
	ADCDamageVolume();

	virtual void Tick(float DeltaSeconds) override;

	/** True while ActiveConditions pass (always, when there are none). */
	UFUNCTION(BlueprintPure, Category="Damage")
	bool IsHazardActive() const;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** Shown once to each pawn the first time it is hurt here. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage", meta=(ClampMin="0"))
	float DamagePerSecond = 20.0f;

	/**
	 *  World conditions (flags, deaths, discovered locations) that must all pass for the hazard to hurt.
	 *  Evaluated against the world, not the victim, so item and quest conditions never pass here.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage")
	TArray<FDCGameplayCondition> ActiveConditions;

private:

	TSet<TWeakObjectPtr<AActor>> WarnedActors;

	bool bShownActive = true;
};
