#include "Items/DCItemDefinition.h"
#include "Core/DCGameplayTags.h"
#include "UObject/UObjectIterator.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "DCItemDefinition"

bool UDCItemDefinition::IsFirearm() const
{
	return Category.MatchesTag(DCTags::Item_Weapon_Firearm);
}

const UDCItemDefinition* UDCItemDefinition::FindByItemId(FName ItemId)
{
	if (ItemId.IsNone())
	{
		return nullptr;
	}

	for (TObjectIterator<UDCItemDefinition> It; It; ++It)
	{
		if (It->HasAnyFlags(RF_ClassDefaultObject | RF_BeginDestroyed | RF_FinishDestroyed))
		{
			continue;
		}
		if (It->ItemId == ItemId)
		{
			return *It;
		}
	}
	return nullptr;
}

const UDCItemDefinition* UDCItemDefinition::ResolveSaved(FName ItemId, const FString& ItemPath)
{
	if (const UDCItemDefinition* Found = FindByItemId(ItemId))
	{
		return Found;
	}

	if (!ItemPath.IsEmpty())
	{
		return LoadObject<UDCItemDefinition>(nullptr, *ItemPath);
	}

	return nullptr;
}

#if WITH_EDITOR
EDataValidationResult UDCItemDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (ItemId.IsNone())
	{
		Context.AddError(LOCTEXT("MissingItemId", "ItemId is required."));
		Result = EDataValidationResult::Invalid;
	}

	if (DisplayName.IsEmpty())
	{
		Context.AddError(LOCTEXT("MissingDisplayName", "DisplayName is required."));
		Result = EDataValidationResult::Invalid;
	}

	if (!Category.IsValid())
	{
		Context.AddError(LOCTEXT("MissingCategory", "Category is required."));
		Result = EDataValidationResult::Invalid;
	}

	if (IsFirearm())
	{
		if (AmmoItem.IsNull())
		{
			Context.AddError(LOCTEXT("MissingAmmo", "Firearms require AmmoItem."));
			Result = EDataValidationResult::Invalid;
		}
		if (MagazineSize < 1)
		{
			Context.AddError(LOCTEXT("BadMag", "Firearms require MagazineSize of at least 1."));
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
