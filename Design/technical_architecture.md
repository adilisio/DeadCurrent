# DEAD CURRENT - Technical Architecture

Living document. Update it whenever a foundational system lands or a convention changes.

## Engine and toolchain

- Unreal Engine 5.8 (installed build, `EngineAssociation` = `5.8`)
- Visual Studio 2026 Build Tools with MSVC 14.51, Windows SDK 10.0.26100
- Git with Git LFS for binary assets (see `.gitattributes`)

## Building from a clean clone

1. `git lfs install` (once per machine), then clone. Make sure LFS objects were pulled (`git lfs pull`).
2. Build the editor target:

   ```
   "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" DeadCurrentEditor Win64 Development -Project="<repo>\DeadCurrent.uproject" -WaitMutex
   ```

   Or right-click `DeadCurrent.uproject` > Generate Visual Studio project files, then build `DeadCurrentEditor` from the IDE.
3. Open `DeadCurrent.uproject`. The editor and game start in `/Game/Maps/Lvl_TestGym`.

## Editor scripts

`Tools/EditorScripts/` holds Python scripts that create or regenerate assets, so generated content can be rebuilt instead of hand-edited. They need the editor-only `PythonScriptPlugin` and `EditorScriptingUtilities` plugins, which the project enables. Run one headless with:

```
"C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "<repo>\DeadCurrent.uproject" -run=pythonscript -script="<repo>\Tools\EditorScripts\<script>.py" -unattended -nullrhi
```

| Script | Does |
| --- | --- |
| `fp02_setup_input.py` | Creates `IA_Sprint` and `IA_Crouch`, maps them in `IMC_Default`, assigns them on `BP_FirstPersonCharacter`. Safe to re-run. |
| `fp02_build_test_gym.py` | Regenerates `/Game/Maps/Lvl_TestGym`. Hand edits to that map are lost on the next run. |
| `inspect_template.py` | Read-only dump of player movement settings, input mappings and level actors. |

## Player controls

| Action | Keyboard / mouse | Gamepad |
| --- | --- | --- |
| Move | WASD, arrow keys | Left stick |
| Look | Mouse | Right stick |
| Jump | Space | A / Cross |
| Sprint (hold, forward only) | Left Shift | Left stick click |
| Crouch (toggle) | Left Ctrl, C | B / Circle |

Movement tuning lives on `BP_FirstPersonCharacter`: normal speed is the movement component's `MaxWalkSpeed`, sprint and crouch view settings are in the character's Movement category.

## Test gym

`Lvl_TestGym` is a greybox movement test map (not World Partition). From the spawn, facing forward:

- center: sprint lane with a floor marker every 5 m
- left: stairs up to 1.8 m platforms with 2 m, 3.5 m and 5 m gaps (5 m should need a sprint)
- right: a 30 degree walkable ramp and a 50 degree ramp that should not be climbable
- far right: ledges at 20, 40, 60, 80 and 110 cm (20 and 40 step up, 60 and 80 need a jump, 110 is out of reach)
- far left: a 1.4 m crouch tunnel, then a 1 m wide, 2.1 m tall doorway into a 1 m corridor

## C++ vs Blueprint / data

- **C++**: foundational systems (interaction, inventory runtime, combat and damage, quests, dialogue runtime, save and world state, AI interfaces, gameplay rules).
- **Blueprints and Data Assets**: content (weapons, NPCs, encounters, quests, dialogue, items, perks, locations, effects).
- A designer should be able to add content without touching foundational C++.
- C++ classes are `abstract` bases when content is expected to subclass them in Blueprint.

## Source layout

Single runtime module `DeadCurrent`. The module root is a public include path, so includes are written as `"Folder/File.h"`.

| Folder | Owns |
| --- | --- |
| `Core/` | Game mode, Gameplay Tag declarations, project-wide framework |
| `Character/` | Player character, player controller, camera manager |
| `Interaction/` | Interaction interface, detection, prompts |
| `Items/` | Item definitions |
| `Inventory/` | Inventory runtime |
| `Combat/` | Weapons, damage processing, health |
| `AI/` | Enemy controllers, perception, behavior |
| `Dialogue/` | Dialogue data and runtime |
| `Save/` | Save game, persistent IDs, persistence interfaces |
| `World/` | Persistent world objects and world state |

Split into more modules only when a boundary is proven (for example an editor-only tools module).

## Content layout

| Folder | Owns |
| --- | --- |
| `Characters/` | Character meshes and animation. `Mannequins/` comes from the UE template packs. |
| `Dialogue/` | Dialogue assets |
| `Environment/` | Environment art and dressing |
| `FirstPerson/` | UE First Person template blueprints and test level (temporary) |
| `Input/` | Input actions and mapping contexts (from the template, now owned by us) |
| `Items/` | Item definition assets |
| `LevelPrototyping/` | Template greybox meshes and materials |
| `Maps/` | Game and test maps |
| `UI/` | Widgets |
| `Weapons/` | Weapon assets. `Pistol/`, `Rifle/`, `GrenadeLauncher/` come from the template packs. |
| `World/` | Persistent world actors and props |

Third-party and template content stays in its original folder: moving `.uasset` files outside the editor breaks references. Move them only with the editor's Move/Fix Up Redirectors.

## Naming conventions

C++:

- Classes use the `DC` prefix after the Unreal type prefix: `ADCPlayerCharacter`, `UDCInventoryComponent`, `FDCItemStack`, `IDCInteractable`, `EDCItemCategory`.
- Files are named after the class without the Unreal prefix: `DCPlayerCharacter.h`.
- Log category: `LogDeadCurrent` (add per-system categories such as `LogDCSave` when a system gets noisy).
- API macro: `DEADCURRENT_API`.

Assets:

| Prefix | Type |
| --- | --- |
| `BP_` | Blueprint class |
| `ABP_` | Animation Blueprint |
| `WBP_` | Widget Blueprint |
| `DA_` | Data Asset |
| `DT_` | Data Table |
| `IA_` / `IMC_` | Input Action / Input Mapping Context |
| `SM_` / `SK_` | Static / Skeletal Mesh |
| `M_` / `MI_` | Material / Material Instance |
| `T_` | Texture |
| `S_` / `SC_` | Sound Wave / Sound Cue |
| `Lvl_` | Level |

## Gameplay Tags

Gameplay Tags are the shared vocabulary between systems. Tags used from C++ are declared as native tags in `Core/DCGameplayTags.h` (`DCTags::Item_Ammo`, etc.). Content-only tags may be added in the editor, which writes them to `Config/DefaultGameplayTags.ini`.

Roots and their meaning:

| Root | Meaning |
| --- | --- |
| `Damage.` | Damage types (`Damage.Ballistic`) |
| `Item.` | Item classification (`Item.Weapon.Firearm`, `Item.Ammo`) |
| `Actor.` | Actor disposition (`Actor.Hostile`, `Actor.Friendly`) |
| `State.` | Transient actor state (`State.Dead`) |
| `Interaction.` | Interaction kinds (`Interaction.Pickup`, `Interaction.Loot`, `Interaction.Talk`) |

Later roots from the long-term plan: `Faction.`, `Skill.`, `Status.`, `Quest.`, `WorldPower.`.

Rules: PascalCase segments, singular nouns, no tag without a consumer.

## Template origin

The project was scaffolded from the UE 5.8 First Person C++ template (Horror and Shooter variants excluded). The template classes were renamed and `Config/DefaultEngine.ini` `[CoreRedirects]` maps the old `/Script/TP_FirstPerson` classes to the new ones so template Blueprints keep working. Once those Blueprints have been re-saved in the editor, the redirects can be removed.

| Template class | Project class |
| --- | --- |
| `ATP_FirstPersonCharacter` | `ADCPlayerCharacter` |
| `ATP_FirstPersonGameMode` | `ADCGameMode` |
| `ATP_FirstPersonPlayerController` | `ADCPlayerController` |
| `ATP_FirstPersonCameraManager` | `ADCPlayerCameraManager` |
