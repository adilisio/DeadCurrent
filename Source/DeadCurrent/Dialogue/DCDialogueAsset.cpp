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
