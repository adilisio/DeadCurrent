#include "Dialogue/DCDialogueComponent.h"
#include "Dialogue/DCDialogueAsset.h"
#include "GameFramework/PlayerController.h"
#include "Core/DCGameplayRules.h"

UDCDialogueComponent::UDCDialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UDCDialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsInDialogue())
	{
		return;
	}

	const APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC)
	{
		return;
	}

	static const FKey ChoiceKeys[] = {
		EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four,
		EKeys::Five, EKeys::Six, EKeys::Seven, EKeys::Eight, EKeys::Nine
	};

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(ChoiceKeys); ++Index)
	{
		if (PC->WasInputKeyJustPressed(ChoiceKeys[Index]))
		{
			SelectChoice(Index);
			break;
		}
	}
}

bool UDCDialogueComponent::StartDialogue(const UDCDialogueAsset* Asset, AActor* InParticipant)
{
	if (IsInDialogue() || !Asset)
	{
		return false;
	}

	ActiveAsset = Asset;
	Participant = InParticipant;
	if (!AdvanceTo(Asset->ResolveEntry(FDCRuleContext::ForActor(GetOwner()))))
	{
		ActiveAsset = nullptr;
		Participant = nullptr;
		CurrentNodeId = NAME_None;
		return false;
	}

	OnDialogueStarted.Broadcast();
	return true;
}

void UDCDialogueComponent::EndDialogue()
{
	if (!IsInDialogue())
	{
		return;
	}

	ActiveAsset = nullptr;
	Participant = nullptr;
	CurrentNodeId = NAME_None;
	OnDialogueEnded.Broadcast();
}

bool UDCDialogueComponent::SelectChoice(int32 ChoiceIndex)
{
	const FDCDialogueNode* Node = GetCurrentNode();
	if (!Node)
	{
		return false;
	}

	const TArray<int32> Visible = GetVisibleChoiceIndices();
	if (!Visible.IsValidIndex(ChoiceIndex))
	{
		return false;
	}

	// Copy what we need first: consequences may change state the node list depends on.
	const FDCDialogueChoice Choice = Node->Choices[Visible[ChoiceIndex]];
	UDCGameplayRules::ApplyConsequences(Choice.Consequences, FDCRuleContext::ForActor(GetOwner()));

	if (Choice.NextNodeId.IsNone())
	{
		EndDialogue();
		return true;
	}

	return AdvanceTo(Choice.NextNodeId);
}

const FDCDialogueNode* UDCDialogueComponent::GetCurrentNode() const
{
	return ActiveAsset ? ActiveAsset->FindNode(CurrentNodeId) : nullptr;
}

TArray<int32> UDCDialogueComponent::GetVisibleChoiceIndices() const
{
	TArray<int32> Visible;
	const FDCDialogueNode* Node = GetCurrentNode();
	if (!Node)
	{
		return Visible;
	}

	const FDCRuleContext Context = FDCRuleContext::ForActor(GetOwner());
	for (int32 Index = 0; Index < Node->Choices.Num(); ++Index)
	{
		if (UDCGameplayRules::CheckConditions(Node->Choices[Index].Conditions, Context))
		{
			Visible.Add(Index);
		}
	}
	return Visible;
}

bool UDCDialogueComponent::AdvanceTo(FName NodeId)
{
	if (!ActiveAsset || !ActiveAsset->FindNode(NodeId))
	{
		return false;
	}

	CurrentNodeId = NodeId;
	OnNodeChanged.Broadcast(NodeId);
	return true;
}
