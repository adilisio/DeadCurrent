#include "Dialogue/DCDialogueAsset.h"
#include "Quest/DCQuestComponent.h"

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

FName UDCDialogueAsset::ResolveEntry(const UDCQuestComponent* Quests) const
{
	for (const FDCDialogueEntry& Entry : Entries)
	{
		if (!FindNode(Entry.NodeId))
		{
			continue;
		}

		if (!Quests)
		{
			if (Entry.Conditions.IsEmpty())
			{
				return Entry.NodeId;
			}
			continue;
		}

		if (Quests->MeetsAll(Entry.Conditions))
		{
			return Entry.NodeId;
		}
	}

	return EntryNodeId;
}
