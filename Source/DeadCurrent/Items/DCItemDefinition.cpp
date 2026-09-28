#include "Items/DCItemDefinition.h"
#include "Core/DCGameplayTags.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "DCItemDefinition"

bool UDCItemDefinition::IsFirearm() const
{
	return Category.MatchesTag(DCTags::Item_Weapon_Firearm);
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
