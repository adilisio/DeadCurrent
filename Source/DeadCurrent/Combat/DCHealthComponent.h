#pragma once

#include "CoreMinimal.h"
#include "Combat/DCDamageable.h"
#include "Components/ActorComponent.h"
#include "DCHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FDCHealthChanged, UDCHealthComponent*, Health, float, NewHealth, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDCDied, UDCHealthComponent*, Health, const FDCDamageInfo&, Damage);

/**
 *  Hit points and death. Put this on anything that can die: the player, enemies, destructibles.
 *  Apply damage with ApplyDamageToActor so firearms, melee and hazards share one path.
 */
UCLASS(ClassGroup=(DeadCurrent), meta=(BlueprintSpawnableComponent))
class DEADCURRENT_API UDCHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDCHealthComponent();

	virtual void BeginPlay() override;

	/** Subtracts health. Returns the amount actually applied. Ignored if already dead. */
	UFUNCTION(BlueprintCallable, Category="Health")
	float ApplyDamage(const FDCDamageInfo& Damage);

	UFUNCTION(BlueprintCallable, Category="Health")
	float Heal(float Amount);

	UFUNCTION(BlueprintCallable, Category="Health")
	void ResetHealth();

	/** Restores health from a save without firing OnDied (so load does not start a respawn). */
	void ApplyLoadedState(float NewHealth, bool bIsDead);

	UFUNCTION(BlueprintPure, Category="Health")
	float GetHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category="Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category="Health")
	float GetHealthPercent() const { return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f; }

	UFUNCTION(BlueprintPure, Category="Health")
	bool IsDead() const { return bDead; }

	/**
	 *  Apply damage to Target: reduces its health component if it has one, then notifies
	 *  IDCDamageable for hit reactions. Returns the health actually removed.
	 */
	UFUNCTION(BlueprintCallable, Category="Damage")
	static float ApplyDamageToActor(AActor* Target, const FDCDamageInfo& Damage);

	UPROPERTY(BlueprintAssignable, Category="Health")
	FDCHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="Health")
	FDCDied OnDied;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health", meta=(ClampMin="1"))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health")
	float CurrentHealth = 100.0f;

private:

	bool bDead = false;
};
