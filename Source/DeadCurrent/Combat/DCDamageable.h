#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/HitResult.h"
#include "UObject/Interface.h"
#include "DCDamageable.generated.h"

/** One application of damage. Firearms fill this out; health (FP-07) will consume it. */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCDamageInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
	float Amount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
	FGameplayTag DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
	TWeakObjectPtr<AActor> Instigator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
	TWeakObjectPtr<AActor> Causer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
	FHitResult Hit;
};

UINTERFACE(BlueprintType)
class DEADCURRENT_API UDCDamageable : public UInterface
{
	GENERATED_BODY()
};

/**
 *  Anything that reacts to hits. The pistol uses this; enemies and the player will in FP-07.
 */
class DEADCURRENT_API IDCDamageable
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Damage")
	void ApplyDamage(const FDCDamageInfo& Damage);
};
