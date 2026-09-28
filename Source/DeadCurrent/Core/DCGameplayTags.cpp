#include "Core/DCGameplayTags.h"

namespace DCTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Ballistic, "Damage.Ballistic", "Damage from bullets and other projectiles");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Environmental, "Damage.Environmental", "Damage from hazards such as spills, fire or cold");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Weapon_Firearm, "Item.Weapon.Firearm", "Item is a firearm");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Ammo, "Item.Ammo", "Item is ammunition");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Consumable_Medical, "Item.Consumable.Medical", "Item heals or treats the user when consumed");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Salvage, "Item.Salvage", "Item is scrap used for crafting, repair or trade");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Actor_Hostile, "Actor.Hostile", "Actor is hostile to the player");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Actor_Friendly, "Actor.Friendly", "Actor is friendly to the player");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Dead, "State.Dead", "Actor is dead");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Pickup, "Interaction.Pickup", "Interaction picks the target up");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Loot, "Interaction.Loot", "Interaction opens the target's inventory");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Talk, "Interaction.Talk", "Interaction starts a conversation");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Inspect, "Interaction.Inspect", "Interaction shows a description of the target");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Use, "Interaction.Use", "Interaction operates the target, such as a door or switch");
}
