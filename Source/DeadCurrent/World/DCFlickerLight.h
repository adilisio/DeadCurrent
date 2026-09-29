#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/DCGameplayTypes.h"
#include "DCFlickerLight.generated.h"

class UMaterialInstanceDynamic;
class UPointLightComponent;
class USoundAttenuation;
class USoundBase;
class UStaticMeshComponent;

/**
 *  A cosmetic light that flickers irregularly: a point light plus an optional glowing mesh whose
 *  material color parameter is scaled with it. Landmarks and hazards use it to draw the eye (a lamp
 *  on a wreck's mast, sparks over live water). No gameplay effect.
 *
 *  ActiveConditions (world conditions) switch it off, e.g. once a hazard's power is cut.
 */
UCLASS()
class DEADCURRENT_API ADCFlickerLight : public AActor
{
	GENERATED_BODY()

public:
	ADCFlickerLight();

	virtual void Tick(float DeltaSeconds) override;

	/** True while ActiveConditions pass (always, when there are none). */
	UFUNCTION(BlueprintPure, Category="Flicker")
	bool IsLightActive() const;

	/** Current brightness, 0 (off) to MaxBrightness. */
	UFUNCTION(BlueprintPure, Category="Flicker")
	float GetBrightness() const { return Brightness; }

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UPointLightComponent> Light;

	/** Optional glowing mesh (no collision). Leave its mesh empty for a light only. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> Glow;

	/** Vector parameter on the glow mesh's material that is scaled with the brightness. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flicker")
	FName GlowColorParameter = TEXT("Color");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flicker", meta=(ClampMin="0", ClampMax="1"))
	float MinBrightness = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flicker", meta=(ClampMin="0"))
	float MaxBrightness = 1.0f;

	/** Chance per change of dropping out completely for a moment. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flicker", meta=(ClampMin="0", ClampMax="1"))
	float DropoutChance = 0.12f;

	/** Seconds between brightness changes, picked at random in this range. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flicker", meta=(ClampMin="0.01", Units="s"))
	float MinInterval = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flicker", meta=(ClampMin="0.01", Units="s"))
	float MaxInterval = 0.6f;

	/** World conditions that must all pass for the light to be on. Evaluated against the world, like hazards. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flicker")
	TArray<FDCGameplayCondition> ActiveConditions;

	/** Optional one-shot snaps. One is picked at random when the light flashes on. Cosmetic only. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flicker|Audio")
	TArray<TSoftObjectPtr<USoundBase>> FlashSounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flicker|Audio")
	TSoftObjectPtr<USoundAttenuation> FlashAttenuation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flicker|Audio", meta=(ClampMin="0"))
	float FlashVolume = 0.22f;

	/** Chance that a flash-on plays a snap, so six lights are not a machine gun. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flicker|Audio", meta=(ClampMin="0", ClampMax="1"))
	float FlashSoundChance = 0.3f;

private:

	void ApplyBrightness(float NewBrightness);

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> GlowMaterial;

	FLinearColor BaseGlowColor = FLinearColor::White;

	float BaseIntensity = 0.0f;

	float Brightness = 1.0f;

	float TimeToNextChange = 0.0f;
};
