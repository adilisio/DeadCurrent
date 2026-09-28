#include "Items/DCItemPickup.h"
#include "Components/StaticMeshComponent.h"
#include "Core/DCGameplayTags.h"
#include "Engine/StaticMesh.h"
#include "Items/DCItemDefinition.h"
#include "UI/DCHUD.h"

#define LOCTEXT_NAMESPACE "DCItemPickup"

ADCItemPickup::ADCItemPickup()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
	SetRootComponent(Mesh);
}

void ADCItemPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyItemMesh();
}

void ADCItemPickup::ApplyItemMesh()
{
	if (!Item)
	{
		return;
	}

	// Pickups are few and small, so a synchronous load keeps placement and spawning simple.
	Mesh->SetStaticMesh(Item->WorldMesh.LoadSynchronous());
	Mesh->SetRelativeScale3D(Item->WorldMeshScale);
}

bool ADCItemPickup::CanInteract_Implementation(AActor* Interactor) const
{
	return Item != nullptr && Quantity > 0;
}

FDCInteractionPrompt ADCItemPickup::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	FText Target = Item ? Item->DisplayName : FText::GetEmpty();
	if (Quantity > 1)
	{
		Target = FText::Format(LOCTEXT("TargetWithQuantity", "{0} ({1})"), Target, Quantity);
	}

	return { LOCTEXT("TakeAction", "Take"), Target };
}

FGameplayTag ADCItemPickup::GetInteractionType_Implementation() const
{
	return DCTags::Interaction_Pickup;
}

void ADCItemPickup::Interact_Implementation(AActor* Interactor)
{
	// TODO(FP-05): add to the interactor's inventory instead of only reporting it.
	const FText Message = Quantity > 1
		? FText::Format(LOCTEXT("TakenQuantity", "Took {0} ({1})"), Item->DisplayName, Quantity)
		: FText::Format(LOCTEXT("Taken", "Took {0}"), Item->DisplayName);
	ADCHUD::ShowMessageFor(Interactor, Message, 2.0f);

	Destroy();
}

#undef LOCTEXT_NAMESPACE
