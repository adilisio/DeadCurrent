#include "World/DCInspectableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Core/DCGameplayRules.h"
#include "Core/DCGameplayTags.h"
#include "UI/DCHUD.h"

#define LOCTEXT_NAMESPACE "DCInspectableActor"

ADCInspectableActor::ADCInspectableActor()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	Action = LOCTEXT("InspectAction", "Inspect");
}

bool ADCInspectableActor::CanInteract_Implementation(AActor* Interactor) const
{
	return !Description.IsEmpty() || !Variants.IsEmpty();
}

const FDCInspectVariant* ADCInspectableActor::FindVariant(AActor* Interactor) const
{
	const FDCRuleContext Context = FDCRuleContext::ForActor(Interactor);
	return Variants.FindByPredicate([&Context](const FDCInspectVariant& Variant)
	{
		return UDCGameplayRules::CheckConditions(Variant.Conditions, Context);
	});
}

FDCInteractionPrompt ADCInspectableActor::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	const FDCInspectVariant* Variant = FindVariant(Interactor);
	return { Variant && !Variant->Action.IsEmpty() ? Variant->Action : Action, DisplayName };
}

FGameplayTag ADCInspectableActor::GetInteractionType_Implementation() const
{
	return DCTags::Interaction_Inspect;
}

void ADCInspectableActor::Interact_Implementation(AActor* Interactor)
{
	if (const FDCInspectVariant* Variant = FindVariant(Interactor))
	{
		ADCHUD::ShowMessageFor(Interactor, Variant->Description, DescriptionDuration);
		UDCGameplayRules::ApplyConsequences(Variant->Consequences, FDCRuleContext::ForActor(Interactor));
		return;
	}

	if (!Description.IsEmpty())
	{
		ADCHUD::ShowMessageFor(Interactor, Description, DescriptionDuration);
	}
}

#undef LOCTEXT_NAMESPACE
