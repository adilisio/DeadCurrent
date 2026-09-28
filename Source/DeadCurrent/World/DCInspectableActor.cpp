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
}

bool ADCInspectableActor::CanInteract_Implementation(AActor* Interactor) const
{
	return !Description.IsEmpty() || !Variants.IsEmpty();
}

FDCInteractionPrompt ADCInspectableActor::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	return { LOCTEXT("InspectAction", "Inspect"), DisplayName };
}

FGameplayTag ADCInspectableActor::GetInteractionType_Implementation() const
{
	return DCTags::Interaction_Inspect;
}

void ADCInspectableActor::Interact_Implementation(AActor* Interactor)
{
	const FDCRuleContext Context = FDCRuleContext::ForActor(Interactor);
	for (const FDCInspectVariant& Variant : Variants)
	{
		if (UDCGameplayRules::CheckConditions(Variant.Conditions, Context))
		{
			ADCHUD::ShowMessageFor(Interactor, Variant.Description, DescriptionDuration);
			UDCGameplayRules::ApplyConsequences(Variant.Consequences, Context);
			return;
		}
	}

	if (!Description.IsEmpty())
	{
		ADCHUD::ShowMessageFor(Interactor, Description, DescriptionDuration);
	}
}

#undef LOCTEXT_NAMESPACE
