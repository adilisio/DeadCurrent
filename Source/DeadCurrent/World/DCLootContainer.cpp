#include "World/DCLootContainer.h"
#include "Components/StaticMeshComponent.h"
#include "Core/DCGameplayTags.h"
#include "Engine/CollisionProfile.h"
#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Save/DCPersistentIdComponent.h"
#include "UI/DCHUD.h"

#define LOCTEXT_NAMESPACE "DCLootContainer"

ADCLootContainer::ADCLootContainer()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	SetRootComponent(Mesh);

	InventoryComponent = CreateDefaultSubobject<UDCInventoryComponent>(TEXT("Inventory"));
	PersistentIdComponent = CreateDefaultSubobject<UDCPersistentIdComponent>(TEXT("PersistentId"));

	DisplayName = LOCTEXT("DefaultName", "Container");
}

bool ADCLootContainer::IsEmpty() const
{
	return !InventoryComponent || InventoryComponent->IsEmpty();
}

FText ADCLootContainer::DescribeStack(const FDCItemStack& Stack)
{
	const FText Name = Stack.Item ? Stack.Item->DisplayName : FText::GetEmpty();
	return Stack.Quantity > 1 ? FText::Format(LOCTEXT("StackQty", "{0} ({1})"), Name, Stack.Quantity) : Name;
}

bool ADCLootContainer::CanInteract_Implementation(AActor* Interactor) const
{
	return InventoryComponent && Interactor && Interactor->FindComponentByClass<UDCInventoryComponent>();
}

FDCInteractionPrompt ADCLootContainer::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	if (IsEmpty())
	{
		return { LOCTEXT("SearchAction", "Search"), FText::Format(LOCTEXT("EmptyTarget", "{0} (empty)"), DisplayName) };
	}

	return { LOCTEXT("TakeAction", "Take"),
		FText::Format(LOCTEXT("TakeTarget", "{0} from {1}"), DescribeStack(InventoryComponent->GetStacks()[0]), DisplayName) };
}

FGameplayTag ADCLootContainer::GetInteractionType_Implementation() const
{
	return DCTags::Interaction_Loot;
}

void ADCLootContainer::Interact_Implementation(AActor* Interactor)
{
	if (IsEmpty())
	{
		ADCHUD::ShowMessageFor(Interactor, FText::Format(LOCTEXT("NothingLeft", "Nothing left in the {0}."), DisplayName), 2.0f);
		return;
	}

	const FText Taken = DescribeStack(InventoryComponent->GetStacks()[0]);
	if (TakeNext(Interactor) <= 0)
	{
		return;
	}

	FText Message;
	if (IsEmpty())
	{
		Message = FText::Format(LOCTEXT("TookLast", "Took {0}. The {1} is empty."), Taken, DisplayName);
	}
	else
	{
		TArray<FText> Left;
		for (const FDCItemStack& Stack : InventoryComponent->GetStacks())
		{
			Left.Add(DescribeStack(Stack));
		}
		Message = FText::Format(LOCTEXT("TookSome", "Took {0}. Still inside: {1}"), Taken,
			FText::Join(LOCTEXT("ListSeparator", ", "), Left));
	}
	ADCHUD::ShowMessageFor(Interactor, Message, 3.0f);
}

int32 ADCLootContainer::TakeNext(AActor* Taker)
{
	UDCInventoryComponent* Destination = Taker ? Taker->FindComponentByClass<UDCInventoryComponent>() : nullptr;
	return Destination && InventoryComponent ? InventoryComponent->TransferFirstStackTo(Destination) : 0;
}

FName ADCLootContainer::GetPersistentId_Implementation() const
{
	return UDCPersistentIdComponent::GetIdOnActor(this);
}

void ADCLootContainer::CapturePersistentState_Implementation(FDCPersistentActorState& OutState) const
{
	OutState.PersistentId = UDCPersistentIdComponent::GetIdOnActor(this);
	OutState.bExists = true;
	OutState.bAlive = true;
	// Contents are captured by UDCSaveSubsystem from the inventory component (flat world-inventory arrays).
}

void ADCLootContainer::ApplyPersistentState_Implementation(const FDCPersistentActorState& State)
{
	// Contents are restored by UDCSaveSubsystem from the flat inventory arrays. An actor that is
	// simply absent from the save keeps its authored state (it was added to the map later).
}

#undef LOCTEXT_NAMESPACE
