#include "World/DCInspectableActor.h"
#include "Components/StaticMeshComponent.h"
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
	return !Description.IsEmpty();
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
	ADCHUD::ShowMessageFor(Interactor, Description, DescriptionDuration);
}

#undef LOCTEXT_NAMESPACE
