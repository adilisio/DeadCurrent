#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DCItemDefinition.generated.h"

class UStaticMesh;
class UTexture2D;

/**
 *  Shared, read-only data for one kind of item. World pickups, inventories and saves refer to items by definition.
 *  Create one Data Asset per item under /Game/Items, named DA_Item_<Name>.
 */
UCLASS(BlueprintType, Const)
class DEADCURRENT_API UDCItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	/** Stable identifier written to save games. Never change it once an item has shipped in a save. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item")
	FName ItemId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item", meta=(MultiLine="true"))
	FText Description;

	/** Item.* tag, e.g. Item.Weapon.Firearm or Item.Ammo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item", meta=(Categories="Item"))
	FGameplayTag Category;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item", meta=(ClampMin="0", Units="kg"))
	float Weight = 0.0f;

	/** Base trade value before merchant and reputation modifiers */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item", meta=(ClampMin="0"))
	int32 Value = 0;

	/** How many fit in one inventory stack. 1 means every item is kept separately. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item", meta=(ClampMin="1"))
	int32 MaxStackSize = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Mesh used when the item lies in the world as a pickup */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	TSoftObjectPtr<UStaticMesh> WorldMesh;

	/** Scale applied to WorldMesh, so generic meshes can stand in for items without their own art */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
	FVector WorldMeshScale = FVector::OneVector;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
