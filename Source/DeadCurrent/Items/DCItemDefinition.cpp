#include "Items/DCItemDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "DCItemDefinition"

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

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
