#include "Dialogue/DCDialogueAsset.h"

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
