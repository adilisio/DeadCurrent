# DEAD CURRENT - Technical Architecture

Living document. Update it whenever a foundational system lands or a convention changes.

## Engine and toolchain

- Unreal Engine 5.8 (installed build, `EngineAssociation` = `5.8`)
- Visual Studio 2026 Build Tools with MSVC 14.51, Windows SDK 10.0.26100
- Git with Git LFS for binary assets (see `.gitattributes`). Assets are not marked `lockable`: LFS makes lockable files read-only until locked, which breaks editor and script saves. Add locking only if more people start editing the same assets.

## Building from a clean clone

1. `git lfs install` (once per machine), then clone. Make sure LFS objects were pulled (`git lfs pull`).
2. Build the editor target:

   ```
   "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" DeadCurrentEditor Win64 Development -Project="<repo>\DeadCurrent.uproject" -WaitMutex
   ```

   Or right-click `DeadCurrent.uproject` > Generate Visual Studio project files, then build `DeadCurrentEditor` from the IDE.
3. Open `DeadCurrent.uproject`. The editor and game start in `/Game/Maps/Lvl_Boathouse`. The systems test gym remains at `/Game/Maps/Lvl_TestGym`.

## Playtesting

`Tools/PlayTest.bat` launches the game standalone (no editor) in a 1280x720 window on the discrete GPU. It forces DX11, Low scalability, 70% resolution, no Lumen / ray tracing / virtual shadows / volumetric clouds / fog / SSAO / bloom / motion blur, a 400 MB texture pool, no vsync, and a 60 FPS cap. Those overrides apply from the first frame (`-dpcvars`); the project's own rendering settings are unchanged. It uses the compiled editor build, so rebuild `DeadCurrentEditor` after C++ changes. The first launch (and the first launch after switching graphics APIs) compiles shaders and takes several minutes.

## Editor scripts

`Tools/EditorScripts/` holds Python scripts that create or regenerate assets, so generated content can be rebuilt instead of hand-edited. They need the editor-only `PythonScriptPlugin` and `EditorScriptingUtilities` plugins, which the project enables. Run one headless with:

```
"C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "<repo>\DeadCurrent.uproject" -run=pythonscript -script="<repo>\Tools\EditorScripts\<script>.py" -unattended -nullrhi
```

| Script | Does |
| --- | --- |
| `setup_player_input.py` | Creates the player input actions (sprint, crouch, interact, inventory, fire, reload, holster), maps them in `IMC_Default`, assigns them on `BP_FirstPersonCharacter`. Safe to re-run; add new player actions here. |
| `create_items.py` | Creates or updates the item definitions in `/Game/Items`. Safe to re-run; edits made in the editor to those items are overwritten. |
| `create_dialogue.py` | Creates or updates dialogue Data Assets in `/Game/Dialogue`. Safe to re-run. |
| `build_test_gym.py` | Regenerates `/Game/Maps/Lvl_TestGym`. Hand edits to that map are lost on the next run. |
| `build_boathouse.py` | Regenerates `/Game/Maps/Lvl_Boathouse`, the first-playable scenario. Hand edits to that map are lost on the next run. |
| `inspect_template.py` | Read-only dump of player movement settings, input mappings and level actors. |

## Player controls

| Action | Keyboard / mouse | Gamepad |
| --- | --- | --- |
| Move | WASD, arrow keys | Left stick |
| Look | Mouse | Right stick |
| Jump | Space | A / Cross |
| Sprint (hold, forward only) | Left Shift | Left stick click |
| Crouch (toggle) | Left Ctrl, C | B / Circle |
| Interact | E | X / Square |
| Inventory (toggle) | Tab, I | View / Back |
| Fire | Left mouse | Right trigger |
| Reload | R | Y / Triangle |
| Holster / draw | 1 | D-pad up |

Movement tuning lives on `BP_FirstPersonCharacter`: normal speed is the movement component's `MaxWalkSpeed`, sprint and crouch view settings are in the character's Movement category.

## Test gym

`Lvl_TestGym` is a greybox movement and interaction test map (not World Partition). Interactables use the colored prototype material. From the spawn, facing forward:

- just ahead: an inspectable crate (right) and sign (left), and a table with the four test item pickups
- center: sprint lane with a floor marker every 5 m
- left: stairs up to 1.8 m platforms with 2 m, 3.5 m and 5 m gaps (5 m should need a sprint)
- right: a 30 degree walkable ramp and a 50 degree ramp that should not be climbable
- far right: ledges at 20, 40, 60, 80 and 110 cm (20 and 40 step up, 60 and 80 need a jump, 110 is out of reach)
- far left: a 1.4 m crouch tunnel, then a 1 m wide, 2.1 m tall doorway with a door, a 2 m clear area for the door to swing into, then a 1 m corridor
- shooting range: a close plate 8 m ahead on the right, three plates at 20 m down the sprint lane, and a backstop behind them
- right of spawn: a chemical-spill pad that damages the player
- past the 20 m plates: a scavenger on a patrol loop
- right-forward of spawn: Mara, a friendly NPC who turns to face you and talks on **E**

## First playable map

`Lvl_Boathouse` is the first-playable scenario (not World Partition). Greybox, same prototype materials as the gym. The player wakes inside the boathouse, takes the pistol from the workbench, goes out the door, meets a scavenger on the shore path, then finds Mara behind a ridge out of the scavenger's sight. Persistent IDs use the `boat.` prefix.

## Interaction

Anything the player can use implements `IDCInteractable` (`Interaction/DCInteractable.h`), in C++ or Blueprint:

- `CanInteract(Interactor)`: unusable interactables show no prompt
- `GetInteractionPrompt(Interactor)`: action and target name, shown as `[E] Open Test Door`
- `GetInteractionType()`: an `Interaction.*` tag
- `Interact(Interactor)`

`UDCInteractorComponent` on the player sweeps from the view point each frame (2.5 m range, 8 cm radius, Visibility channel), keeps the usable interactable in focus, and broadcasts `OnFocusChanged`. The interact input calls `TryInteract()`. `ADCHUD` draws the prompt from the focused actor, so new interactable types need no UI or player changes.

Current implementations: `ADCInspectableActor` (shows a description), `ADCDoor` (swings away from the user), `ADCItemPickup` (adds to inventory), `ADCScavengerCharacter` (loot after death), `ADCFriendlyNPC` (talk).

## Items

Each kind of item is a `UDCItemDefinition` Data Asset in `/Game/Items`, named `DA_Item_<Name>`. It holds:

- `ItemId`: the stable identifier saves will store. Never change it after an item exists in a save.
- `DisplayName`, `Description`, `Category` (an `Item.*` tag), `Weight` (kg), `Value`, `MaxStackSize`
- `Icon`, `WorldMesh`, `WorldMeshScale` (lets generic meshes stand in until an item has its own art)

The editor's data validation flags definitions missing an ID, name or category.

`ADCItemPickup` is an item lying in the world. It references a definition and a quantity, and takes its mesh and prompt text (`[E] Take 9mm Rounds (24)`) from the definition. Taking it adds it to the interactor's inventory; it is only usable by actors that have one.

| Asset | ItemId | Category | Stack |
| --- | --- | --- | --- |
| `DA_Item_Pistol` | `pistol_service` | `Item.Weapon.Firearm` | 1 |
| `DA_Item_Ammo9mm` | `ammo_9mm` | `Item.Ammo` | 999 |
| `DA_Item_FieldDressing` | `field_dressing` | `Item.Consumable.Medical` | 10 |
| `DA_Item_SalvagedWiring` | `salvage_wiring` | `Item.Salvage` | 50 |

`DA_Item_Pistol` is a firearm item: same `UDCItemDefinition` as other items, with magazine, ammo, damage, recoil and equipped-view fields. `IsFirearm()` is true when `Category` is `Item.Weapon.Firearm`. Other guns are more of these assets, not new C++ classes.

## Weapons

`ADCFirearm` (`Combat/`) is the equipped gun. The player auto-equips the first firearm that enters inventory, and **1** holsters or draws it. Firing is hitscan from the view point (WorldStatic, WorldDynamic and Pawn, 100 m). Pawn capsules ignore the Visibility channel, so the gun does not use it. Magazine rounds are separate from inventory; **R** moves ammo from inventory into the magazine after `ReloadDuration`. Dry-fire shows "Reload" or "No ammo".

Recoil kicks the view up (with a little random yaw) and recovers over a few frames. Hits spawn a short-lived impact mark, then `UDCHealthComponent::ApplyDamageToActor` applies `FDCDamageInfo`. Actors may also implement `IDCDamageable` for hit reactions (flashes, messages).

## Health

`UDCHealthComponent` (`Combat/`) is the reusable hit-point pool. The player, range plates and the scavenger all have one. `ApplyDamage` / `Heal` / `ResetHealth`, `OnHealthChanged`, `OnDied`. Dead actors ignore further damage until `ResetHealth`.

The pistol does 25, default max health is 100, so four hits drop a plate. `ADCShootableTarget` flashes and shows remaining health, then falls over on death. The player shows a health bar (bottom left); on death, movement is disabled and they respawn at the PlayerStart after 4 seconds with full health (inventory is kept).

`ADCDamageVolume` (`World/`) is an overlap pad that applies `Damage.Environmental` each second. The gym has a "chemical spill" at 25/s, so standing on it kills in about 4 seconds.

Automation test `DeadCurrent.Combat.Health` covers damage, death, ignored extra hits, heal and reset.

The HUD shows `magazine | reserve` in the bottom right while a gun is drawn, `Reloading` during reload, and `[R] Reload` when the mag is empty and reserve remains.

Automation test `DeadCurrent.Combat.FirearmAmmo` covers magazine fill, consumption and partial reload.

## AI

`ADCScavengerCharacter` (`AI/`) is a mannequin pawn with health and an inventory. After death the corpse implements `IDCInteractable` (`Interaction.Loot`): **E** transfers every stack into the player's inventory and the prompt goes away when empty. Starting loot (12× 9mm, a field dressing, 2× salvaged wiring) is granted on first BeginPlay if the inventory was authored empty. `ADCScavengerController` runs a small state machine:

- **Patrol** between authored world points
- **Chase** when sight (18 m, 75° cone) picks up the player, or when shot
- **Attack** melee (15 damage, 1.3 s cooldown, 1.7 m range) using `ApplyDamageToActor` with `Damage.Melee`
- **Investigate** last seen location after 4 s without sight, then back to patrol
- **Dead** on `OnDied`: movement off, ragdoll, controller detached

The gym scavenger patrols a square past the 20 m plates. A floating state label (`Patrol` / `Chase` / …) is drawn above their head for playtests (`bDrawState`). The player registers as a sight stimulus so perception can see them.

The test gym includes a `NavMeshBoundsVolume` covering the floor. Nav rebuilds at runtime if the saved mesh is empty. Rebuild the gym script after C++ AI changes so the scavenger is placed on the nav mesh.

## NPCs

`ADCFriendlyNPC` (`AI/`) is an idle character (no combat brain). She blocks Visibility so **E** can talk (`Interaction.Talk`), turns to face the player within 6 m, and starts `DA_Dialogue_MaraIntro` on the player's dialogue component. The gym places Mara at (350, 1100), upright (spawn uses named Rotator pitch/yaw/roll — positional Rotator was pitching her over).

## Dialogue

`UDCDialogueAsset` (`Dialogue/`) is a node graph Data Asset under `/Game/Dialogue`. Each node has a speaker, line, and choices. A choice's `NextNodeId` is empty to end the conversation. Conditions and consequences can hang on choices later without changing the walker.

`UDCDialogueComponent` on the player runs the active conversation: `StartDialogue` / `SelectChoice` / `EndDialogue`. Digit keys **1–9** pick choices. The canvas HUD draws the current line and numbered replies. Fire, inventory and interact are blocked while talking.

Automation test `DeadCurrent.Dialogue.Branching` covers start, branch, and goodbye.

## Inventory

`UDCInventoryComponent` (`Inventory/`) holds a list of `FDCItemStack` (definition + quantity). The player has one; corpses and containers will use the same component, with starting contents set on the placed actor's `Stacks`.

- `AddItem(Item, Quantity)` tops up existing stacks, then starts new ones, never exceeding `MaxStackSize`. Returns the amount added. There is no capacity or weight limit yet.
- `RemoveItem(Item, Quantity)` takes from the newest stacks first and drops empty stacks. Returns the amount removed.
- `TransferAllTo(Destination)` moves every stack into another inventory (corpses, later containers).
- `GetQuantity(Item)`, `GetTotalWeight()`, `GetStacks()`, `IsEmpty()`
- `OnInventoryChanged` fires after every change (for UI, and later saving)

Saves will store each stack as `ItemId` + quantity.

Automation test `DeadCurrent.Inventory.Stacking` covers stacking and removal. Run it from the Session Frontend, or headless:

```
UnrealEditor-Cmd.exe DeadCurrent.uproject -unattended -nullrhi -nosound "-ExecCmds=Automation RunTests DeadCurrent; Quit" -TestExit="Automation Test Queue Empty"
```

## Save

`UDCPersistentIdComponent` (`Save/`) holds an authored `FName` unique within the map (e.g. `gym.scavenger`, `gym.mara`). `UDCPersistentRegistry` is a world subsystem that registers those IDs at BeginPlay and can `FindActor` by id. Duplicate ids log an error and are rejected.

`IDCPersistent` is the save hook: `GetPersistentId`, `CapturePersistentState`, `ApplyPersistentState`. The scavenger, Mara, the gym door and world pickups implement it. Capture stores id, existence, alive/dead, door yaw, and inventory stacks. Cyan debug labels draw the id above those actors (`bDrawId`).

Gym IDs: `gym.scavenger`, `gym.mara`, `gym.door`, `gym.pickup_pistol`, `gym.pickup_ammo`, `gym.pickup_dressing`, `gym.pickup_wiring`.

Boathouse IDs: `boat.scavenger`, `boat.mara`, `boat.door`, `boat.pickup_pistol`, `boat.pickup_ammo`, `boat.pickup_dressing`.

`UDCSaveGame` is the slot (`DeadCurrent`, user 0). `UDCSaveSubsystem` (`UGameInstanceSubsystem`) writes player transform, health, inventory, equipped magazine, and every registered persistent actor. F5 saves, F9 loads. Load applies world actors first (destroy pickups missing from the save, restore scavenger death/loot and door swing), then replaces player inventory without triggering a magazine refill from reserve. The first F9 after a process start can hitch while item assets resolve; polish that in FP-15.

Automation tests `DeadCurrent.Save.PersistentId` and `DeadCurrent.Save.InventoryRestore` cover lookup and inventory snapshot restore.

`ADCHUD` is a temporary canvas HUD: crosshair dot, interaction prompt, timed messages via `ADCHUD::ShowMessageFor`, the inventory panel, the weapon ammo readout, a health bar, and the dialogue panel. It will be replaced by UMG widgets when the HUD grows.

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
| `UI/` | HUD and widget base classes |
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
| `Damage.` | Damage types (`Damage.Ballistic`, `Damage.Environmental`, `Damage.Melee`) |
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
