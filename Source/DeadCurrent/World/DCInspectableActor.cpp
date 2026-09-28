#include "World/DCInspectableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Core/DCGameplayTags.h"
#include "Quest/DCQuestComponent.h"
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
	FText Line = Description;
	if (!WorldFlag.IsNone() && !FlagDescription.IsEmpty() && Interactor)
	{
		if (const UDCQuestComponent* Quests = Interactor->FindComponentByClass<UDCQuestComponent>())
		{
			if (Quests->HasFlag(WorldFlag))
			{
				Line = FlagDescription;
			}
		}
	}

	ADCHUD::ShowMessageFor(Interactor, Line, DescriptionDuration);
}

#undef LOCTEXT_NAMESPACE
