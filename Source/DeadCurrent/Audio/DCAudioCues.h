#pragma once

#include "CoreMinimal.h"

class UObject;

/**
 *  First-pass presentation cues that follow an existing action (a pickup taken, the inventory opening).
 *  Each plays a sound asset by path. A missing asset is a silent no-op, the way an empty firearm sound is,
 *  so a build without the audio content still runs. These paths are in /Game/Interface_And_Item_Sounds, which the project
 *  always cooks with /Game/Audio (Config/DefaultGame.ini).
 */
namespace DCAudioCues
{
	inline constexpr const TCHAR* Pickup = TEXT("/Game/Interface_And_Item_Sounds/Cues/Click_03_Cue.Click_03_Cue");
	inline constexpr const TCHAR* InventoryOpen = TEXT("/Game/Interface_And_Item_Sounds/Cues/Flick_Switch_01_Cue.Flick_Switch_01_Cue");

	/** Play a non-positional cue for the local player. */
	DEADCURRENT_API void PlayUI(const UObject* WorldContext, const TCHAR* AssetPath, float Volume = 1.0f);
}
