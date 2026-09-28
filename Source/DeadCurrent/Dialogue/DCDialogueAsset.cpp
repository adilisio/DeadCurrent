#include "Dialogue/DCDialogueAsset.h"
#include "Core/DCGameplayRules.h"

const FDCDialogueNode* UDCDialogueAsset::FindNode(FName NodeId) const
{
	if (NodeId.IsNone())
	{
		return nullptr;
	}

	for (const FDCDialogueNode& Node : Nodes)
	{
		if (Node.NodeId == NodeId)
		{
			return &Node;
		}
	}
	return nullptr;
}

FName UDCDialogueAsset::ResolveEntry(const FDCRuleContext& Context) const
{
	for (const FDCDialogueEntry& Entry : Entries)
	{
		if (FindNode(Entry.NodeId) && UDCGameplayRules::CheckConditions(Entry.Conditions, Context))
		{
			return Entry.NodeId;
		}
	}

	return EntryNodeId;
}

#if WITH_EDITOR
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "DCDialogueAsset"

EDataValidationResult UDCDialogueAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	auto Fail = [&Context, &Result](const FText& Message)
	{
		Context.AddError(Message);
		Result = EDataValidationResult::Invalid;
	};

	if (!FindNode(EntryNodeId))
	{
		Fail(FText::Format(LOCTEXT("BadEntry", "EntryNodeId {0} is not a node."), FText::FromName(EntryNodeId)));
	}

	for (const FDCDialogueEntry& Entry : Entries)
	{
		if (!FindNode(Entry.NodeId))
		{
			Fail(FText::Format(LOCTEXT("BadEntryNode", "Entry points at missing node {0}."), FText::FromName(Entry.NodeId)));
		}
	}

	TSet<FName> Seen;
	for (const FDCDialogueNode& Node : Nodes)
	{
		if (Node.NodeId.IsNone() || Seen.Contains(Node.NodeId))
		{
			Fail(FText::Format(LOCTEXT("BadNodeId", "Node id '{0}' is empty or duplicated."), FText::FromName(Node.NodeId)));
		}
		Seen.Add(Node.NodeId);

		// A node whose choices can all be hidden would trap the player in the conversation.
		const bool bHasUnconditionalChoice = Node.Choices.ContainsByPredicate(
			[](const FDCDialogueChoice& Choice) { return Choice.Conditions.IsEmpty(); });
		if (!bHasUnconditionalChoice)
		{
			Fail(FText::Format(LOCTEXT("Trap", "Node {0} has no choice without conditions; the player could be stuck."), FText::FromName(Node.NodeId)));
		}

		for (const FDCDialogueChoice& Choice : Node.Choices)
		{
			if (!Choice.NextNodeId.IsNone() && !FindNode(Choice.NextNodeId))
			{
				Fail(FText::Format(LOCTEXT("BadNext", "Node {0} has a choice to missing node {1}."),
					FText::FromName(Node.NodeId), FText::FromName(Choice.NextNodeId)));
			}
		}
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE
#endif
