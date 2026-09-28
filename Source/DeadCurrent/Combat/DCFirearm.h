#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DCFirearm.generated.h"

class UDCItemDefinition;
class UDCInventoryComponent;
class UPointLightComponent;
class UStaticMeshComponent;

/**
 *  Equipped firearm. Stats come from a firearm item definition in the owner's inventory.
 *  Hitscan: traces from the owner's view, consumes magazine rounds, reloads from inventory ammo.
 */
UCLASS()
class DEADCURRENT_API ADCFirearm : public AActor
{
	GENERATED_BODY()

public:
	ADCFirearm();

	UFUNCTION(BlueprintCallable, Category="Weapon")
	void SetDefinition(const UDCItemDefinition* NewDefinition);

	const UDCItemDefinition* GetDefinition() const { return Definition; }

	UFUNCTION(BlueprintCallable, Category="Weapon")
	void SetHolstered(bool bInHolstered);

	UFUNCTION(BlueprintPure, Category="Weapon")
	bool IsHolstered() const { return bHolstered; }

	UFUNCTION(BlueprintPure, Category="Weapon")
	bool IsReloading() const { return bReloading; }

	UFUNCTION(BlueprintPure, Category="Weapon")
	int32 GetRoundsInMagazine() const { return RoundsInMagazine; }

	UFUNCTION(BlueprintPure, Category="Weapon")
	int32 GetMagazineSize() const;

	UFUNCTION(BlueprintPure, Category="Weapon")
	int32 GetReserveAmmo() const;

	/** True if a shot was fired. Dry-fire and blocked shots return false. */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	bool Fire();

	UFUNCTION(BlueprintCallable, Category="Weapon")
	bool StartReload();

	/** Pulls rounds from Inventory into the magazine immediately. Used by reload and by tests. */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	int32 LoadRoundsFromInventory(UDCInventoryComponent* Inventory);

	/** Removes one round from the magazine. Returns 1 if a round was there. */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	int32 ConsumeRound();

	UFUNCTION(BlueprintPure, Category="Weapon")
	bool CanFire() const;

	UFUNCTION(BlueprintPure, Category="Weapon")
	bool CanReload() const;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UPointLightComponent> MuzzleLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<const UDCItemDefinition> Definition;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	int32 RoundsInMagazine = 0;

private:

	void ApplyEquippedTransform();

	void HideMuzzleLight();

	void FinishReload();

	void NotifyStateChanged() const;

	UDCInventoryComponent* FindOwnerInventory() const;

	bool bHolstered = false;

	bool bReloading = false;

	double LastFireTime = -1000.0;

	FTimerHandle ReloadTimer;

	FTimerHandle MuzzleTimer;
};
