#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/DCGameplayTypes.h"
#include "DCConditionalAudio.generated.h"

class UAudioComponent;
class USoundAttenuation;
class USoundBase;

UENUM(BlueprintType)
enum class EDCConditionalAudioMode : uint8
{
	/** Plays (a looping sound loops) while the conditions pass and is silent otherwise. */
	WhileTrue,
	/** Plays once when the conditions go from failing to passing. Passing at level start is silent. */
	OnceWhenTrue
};

/**
 *  A cosmetic sound that follows the shared rule language. It evaluates the same FDCGameplayCondition
 *  list as dialogue, quests, and hazards, against the local player's pawn when there is one (so HasItem
 *  and ActorDead work) and against this actor otherwise. It is presentation, like ADCFlickerLight: it
 *  sets no flag, applies no consequence, and saves nothing. A save restore that replaces flags silently
 *  is picked up by the periodic re-check.
 *
 *  With no conditions and no attenuation it is an always-on 2D bed (shore wind and water). With an
 *  attenuation asset it is a positional loop (the relay, the live water).
 */
UCLASS()
class DEADCURRENT_API ADCConditionalAudio : public AActor
{
	GENERATED_BODY()

public:
	ADCConditionalAudio();

	virtual void Tick(float DeltaSeconds) override;

	/** True while Conditions pass against the current context (always, when there are none). */
	UFUNCTION(BlueprintPure, Category="Audio")
	bool ConditionsPass() const;

	/** Result of the last evaluation: the sound is (or, for OnceWhenTrue, was just) allowed to play. */
	UFUNCTION(BlueprintPure, Category="Audio")
	bool IsAudible() const { return bAudible; }

	/** How many times a OnceWhenTrue sound has fired (counted even when no sound asset is set). */
	UFUNCTION(BlueprintPure, Category="Audio")
	int32 GetTriggerCount() const { return TriggerCount; }

	/** Evaluate now (Tick calls this every CheckInterval). Public so tests can step it. */
	void Evaluate();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UAudioComponent> Audio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio")
	TSoftObjectPtr<USoundBase> Sound;

	/** Optional. Leave empty for a 2D sound. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio")
	TSoftObjectPtr<USoundAttenuation> Attenuation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0"))
	float VolumeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio")
	EDCConditionalAudioMode Mode = EDCConditionalAudioMode::WhileTrue;

	/** All must pass. World, item, and actor conditions all work. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio")
	TArray<FDCGameplayCondition> Conditions;

	/** Seconds between checks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0.05"))
	float CheckInterval = 0.25f;

	/** Seconds a loop takes to fade out when the conditions stop passing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0"))
	float FadeOutSeconds = 0.4f;

private:

	bool bAudible = false;
	bool bPrimed = false;
	bool bFadingOut = false;
	int32 TriggerCount = 0;
	float TimeToCheck = 0.0f;
};
