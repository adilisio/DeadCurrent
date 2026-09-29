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

First Playable (FirstPhasePlan §18 / §20) is accepted: boathouse loop, per-stack corpse loot, and F5/F9 world restore including remaining corpse stacks.

Micro RPG (LongTermPlan Phase 2) is implemented (Shore Watch played through in the Phase 3 playtest, no issues): the Shore Watch quest in `Lvl_Boathouse` with a combat route and a coil (stealth) route, different outcomes, and save/load at every stage. Exploration Loop (LongTermPlan Phase 3) is implemented and awaiting a human playtest: the Wrecked Survey Launch, an optional point of interest west of the boathouse (location discovery, environmental clues, live-water hazard, two loot containers, one line from Mara). See `Design/CLAUDE_SESSION_REPORT.md` for the playtest checklist and `ExplorationLoopPlan.txt` for the scope.

## Automated tests

```
Tools\RunTests.bat              every test: editor-context suite, then the in-map suite
Tools\RunTests.bat Quest        only DeadCurrent.Quest.*
Tools\RunTests.bat Map          only the in-map suite
Tools\RunTests.bat -nomap       skip the in-map suite
Tools\RunTests.bat -build       build DeadCurrentEditor first
```

Logs go to `Saved/Logs/RunTests.log` and `RunTests_Map.log`. 30 tests: 25 editor-context and 5 in-map; the full run takes about 2 minutes including editor start-up.

| Group | Covers |
| --- | --- |
| `DeadCurrent.Rules.*` | Every condition and consequence type, lists, empty contexts, reference validation |
| `DeadCurrent.Quest.*` | Stages, start stage, outcomes, event-driven transitions (death, item), branch order, one quest moving another, save/restore of progress, stale stages |
| `DeadCurrent.Dialogue.*` | Graph walking, conditional entries, hidden choices, choice consequences (quest, items, flags) |
| `DeadCurrent.Content.Validate` | Every quest and dialogue asset: graph checks and references to real quests, stages and items; Asset Manager registration |
| `DeadCurrent.Content.ShoreWatch.*` | The shipped quest and dialogue assets through both routes, pre-quest shortcuts, the clue line and epilogues, with save/restore at each stage |
| `DeadCurrent.Exploration.Discovery` | Once-only location discovery, rotated volume, announce count, save round-trip, silent restore, pre-Phase-3 save |
| `DeadCurrent.Exploration.Container` | Prompt text, partial and full looting, slot round-trip into a fresh world, a container newer than the save |
| `DeadCurrent.Exploration.WorldConditions` | Damage volume and flicker light switched by a world flag, including silent restore |
| `DeadCurrent.Content.Exploration.MaraWreckLine` | The shipped dialogue: Mara's wreck exchange across Shore Watch states, offered once, survives save/reload |
| `DeadCurrent.Map.Boathouse.*` | Game context, real `Lvl_Boathouse`: placed actors, real interactions and damage, real F9 loads that reopen the map (pre-quest, ready to turn in, complete, legacy save). `SurveyLaunch`: walk-in discovery, live-water damage, clues, partial loot, pull the leads, hidden kit, save, diverge, F9, no re-announce. `SurveyLaunchSaves`: the POI combined with Shore Watch accepted or complete. Scratch save slot |
| `DeadCurrent.World.InspectVariants`, `.Save.*`, `.Inventory.*`, `.Combat.*` | Inspectable variants (including per-variant verbs) and the first-playable systems |

`Core/DCTestHelpers.h` has `FDCTestWorld`, a throwaway game world with subsystems and BeginPlay, for tests that need a registry, world state or component events. In-map tests (`EAutomationTestFlags::ClientContext` only) run under `-game`; with rendering enabled the coil-route test also saves `Saved/Screenshots/<platform>/DC_QuestHUD.png`.

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
| `create_quest.py` | Creates or updates quest Data Assets in `/Game/Quests` (Shore Watch). Safe to re-run. |
| `build_test_gym.py` | Regenerates `/Game/Maps/Lvl_TestGym`. Hand edits to that map are lost on the next run. |
| `build_boathouse.py` | Regenerates `/Game/Maps/Lvl_Boathouse`, the first-playable scenario, Shore Watch and the Survey Launch POI. Also creates the material instances in `/Game/Environment/Materials` (`MI_DC_*`, children of `M_FlatCol` and our own `M_DC_Glow`, an unlit translucent glow with `Color` rgb = emissive, a = opacity). Hand edits to that map are lost on the next run. |
| `inspect_template.py` | Read-only dump of player movement settings, input mappings and level actors. |

`Tools\RebuildContent.bat` runs `create_items`, `create_quest`, `create_dialogue`, `build_test_gym` and `build_boathouse` in that order (dialogue references items; maps reference everything). `Tools\RebuildContent.bat create_quest` runs one. Close the editor first and rebuild C++ before running it. Logs: `Saved/Logs/RebuildContent_<script>.log`.

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
| Holster / draw (not while talking) | 1 | D-pad up |
| Dialogue reply | 1-9 | |
| Save / load | F5 / F9 | |

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
- past the 20 m plates: a scavenger on a patrol loop (`gym.scavenger`), with a relay coil pickup at his camp
- right-forward of spawn: Mara, a friendly NPC who turns to face you and talks on **E**. She uses the same dialogue as the boathouse. Shore Watch's kill route checks `boat.scavenger`, so in the gym only the coil route completes it.

## First playable map

`Lvl_Boathouse` is the first-playable scenario (not World Partition). Greybox, same prototype materials as the gym. The player wakes inside the boathouse, takes the pistol from the workbench, goes out the door, meets a scavenger on the shore path, then finds Mara behind a ridge out of the scavenger's sight. Persistent IDs use the `boat.` prefix.

**Shore Watch** (`shore.watch`, `DA_Quest_ShoreWatch`, `DA_Dialogue_MaraIntro`): the scavenger has wired an old Maritime Authority relay at his camp and it has started transmitting. Mara wants it quiet.

| Stage | Objective / meaning | Leaves when |
| --- | --- | --- |
| `accepted` | Kill him, or pull the coil from his rig | `ActorDead(boat.scavenger)` → `return_killed`; `HasItem(radio_coil)` → `return_coil` (first listed wins) |
| `return_killed` | Tell Mara | Mara's turn-in reply → `done_killed` (+24× 9mm) |
| `return_coil` | Bring Mara the coil | Mara's turn-in reply → `done_coil` (coil taken, +2 field dressings) |
| `done_killed` (outcome) | OnEnter: flag `shore.path_cleared` | |
| `done_coil` (outcome) | OnEnter: flag `shore.relay_recovered`; the scavenger is still alive | |

The relay rig (`RelayRig`, at the camp) is an inspectable with variants: inspecting it while it is live sets `shore.relay_inspected`, which opens a line with Mara (`shore.voice_discussed` once told). Mara's greeting depends on the stage, handles the scavenger already dead (`already_dead`) or the coil already taken before meeting her, takes a coil handed in after the kill route (sets `shore.relay_recovered`), and remarks once if he is killed after the coil route (`shore.mara_heard_kill`). The lookout crate by Mara reads differently for each outcome flag.

### Point of interest: the Wrecked Survey Launch (Exploration Loop)

An optional site on the west shore, not on any route and with no marker. Coordinates are cm; +X is out the boathouse door and the lake is -Y. The bow sits on the stones near (-1500, -300) and the stern is in the water near (-1500, -1260). The site is built by `build_survey_launch()` in `build_boathouse.py` and takes its ids from constants at the top of that script.

- **Notice:** a west window in the boathouse back wall (inspect it: "Look out") mentions a mast and a light. Outside, a leaning mast (about 12.6 m) with a flickering amber lamp (`ADCFlickerLight`) shows above the boathouse roof from the path.
- **Discovery:** an `ADCLocationVolume` (`shore.survey_launch`, "Wrecked Survey Launch") spanning X -2600..-700, Y -1850..450. Walking round the lake side of the boathouse and west triggers it about 7 m past the back wall.
- **Clues** (all `ADCInspectableActor` with variants): name board, life jackets, depth sounder, survey log (verb "Read", sets `wreck.log_read`), breaker panel (changes after the log is read), battery bank (first inspect sets `wreck.battery_seen`; the verb then becomes "Pull the leads", which sets `wreck.power_cut`), emergency beacon (reacts to `shore.relay_inspected` and to the power being cut), dead fish, and the west window (changes after discovery and after the power is cut).
- **Hazard:** the water is a permanent dark, no-collision `WaterSurface` slab. Over it, `LiveWater` is an `ADCDamageVolume` (X -2020..-980, Y -1780..-1000, 20/s, `ActiveConditions = [!wreck.power_cut]`) whose bright blue glow mesh is the "electric" layer, with six flickering spark lights. A ring of pale dead fish lies on the surface at its edge, and a chalked warning plank (`Chalk warning`) stands on the beach at the edge. Pulling the leads switches the damage, the glow and the sparks off and persists through saves; the water itself stays (playtest feedback: it must not vanish).
- **Loot** (`ADCLootContainer`): `boat.wreck_locker` (survey locker in the wheelhouse: 12× 9mm, 3× Salvaged Wiring, 1× Field Dressing) and `boat.wreck_tender` (a small boat about 4 m off the stern, inside the live water, tied to the transom by a line: 1× Sounder Chart, 2× Field Dressing, 18× 9mm; the chart is the site's novel item). The log mentions the tender.
- **Mara:** once `wreck.log_read` is set, her greeting, who, place, in-progress and epilogue nodes (never the turn-ins) offer "There's a wrecked survey launch west of the boathouse. I read her log." She answers; the player can ask "Was it a storm?" (sets `wreck.mara_told`). Offered once.
- **World state ids:** location `shore.survey_launch`; flags `wreck.log_read`, `wreck.battery_seen`, `wreck.power_cut`, `wreck.mara_told`. Never rename a shipped id.

The POI adds no items, quests or C++ specific to the wreck: it is built from the generic classes below. Shore Watch content and `ADCPlayerCharacter` are unchanged.

## Interaction

Anything the player can use implements `IDCInteractable` (`Interaction/DCInteractable.h`), in C++ or Blueprint:

- `CanInteract(Interactor)`: unusable interactables show no prompt
- `GetInteractionPrompt(Interactor)`: action and target name, shown as `[E] Open Test Door`
- `GetInteractionType()`: an `Interaction.*` tag
- `Interact(Interactor)`

`UDCInteractorComponent` on the player sweeps from the view point each frame (2.5 m range, 8 cm radius, Visibility channel), keeps the usable interactable in focus, and broadcasts `OnFocusChanged`. The interact input calls `TryInteract()`. `ADCHUD` draws the prompt from the focused actor, so new interactable types need no UI or player changes.

Current implementations: `ADCInspectableActor` (shows a description; `Variants` pick the text by condition and can run consequences, e.g. a clue flag on first inspection; `Action` is the prompt verb, default "Inspect", and a variant can override it, e.g. "Read" or "Pull the leads"; `GetDisplayName()`), `ADCDoor` (swings away from the user), `ADCItemPickup` (adds to inventory), `ADCLootContainer` (take stacks from a placed container), `ADCScavengerCharacter` (loot after death), `ADCFriendlyNPC` (talk).

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
| `DA_Item_RadioCoil` | `radio_coil` | `Item.Quest` | 1 |
| `DA_Item_SurveyChart` | `survey_chart` | `Item.Quest` | 1 |

(`DA_Item_RadioCoil` displays as "Relay Coil"; its id stays `radio_coil`.)

`DA_Item_SurveyChart` (`survey_chart`, "Sounder Chart", `Item.Quest`, stack 1) is the Survey Launch's novel item, found in the tender. It has no use yet; it exists to be found, read in the inventory, and to be a hook for later phases.

Items, quests and dialogue are registered as Asset Manager primary asset types in `Config/DefaultGame.ini` (`DCItemDefinition`, `DCQuestDefinition`, `DCDialogueAsset`, cook rule AlwaysCook) because gameplay finds them by id, not by hard reference. `UDCContentSubsystem` (`Core/`, game instance) loads every item and quest definition under `/Game/Items` and `/Game/Quests` at startup and keeps them loaded, so `FindByItemId` / `FindByQuestId` work anywhere (including corpse loot after a load) with no hard-coded paths. New definitions in those folders need no code.

`DA_Item_Pistol` is a firearm item: same `UDCItemDefinition` as other items, with magazine, ammo, damage, recoil and equipped-view fields. `IsFirearm()` is true when `Category` is `Item.Weapon.Firearm`. Other guns are more of these assets, not new C++ classes.

## Weapons

`ADCFirearm` (`Combat/`) is the equipped gun. The player auto-equips the first firearm that enters inventory, and **1** holsters or draws it. Firing is hitscan from the view point (WorldStatic, WorldDynamic and Pawn, 100 m). Pawn capsules ignore the Visibility channel, so the gun does not use it. Magazine rounds are separate from inventory; **R** moves ammo from inventory into the magazine after `ReloadDuration`. Dry-fire shows "Reload" or "No ammo".

Recoil kicks the view up (with a little random yaw) and recovers over a few frames. Hits spawn a short-lived impact mark, then `UDCHealthComponent::ApplyDamageToActor` applies `FDCDamageInfo`. Actors may also implement `IDCDamageable` for hit reactions (flashes, messages).

## Health

`UDCHealthComponent` (`Combat/`) is the reusable hit-point pool. The player, range plates and the scavenger all have one. `ApplyDamage` / `Heal` / `ResetHealth`, `OnHealthChanged`, `OnDied`. Dead actors ignore further damage until `ResetHealth`. A death also calls `UDCWorldStateSubsystem::NotifyChanged`, so quests waiting on `ActorDead` advance without the dying actor knowing about quests.

The pistol does 25, default max health is 100, so four hits drop a plate. `ADCShootableTarget` flashes and shows remaining health, then falls over on death. The player shows a health bar (bottom left); on death, movement is disabled and they respawn at the PlayerStart after 4 seconds with full health (inventory is kept).

`ADCDamageVolume` (`World/`) is an overlap pad that applies `Damage.Environmental` each second. The gym has a "chemical spill" at 25/s, so standing on it kills in about 4 seconds. `ActiveConditions` (world conditions only: flags and discovered locations; they are evaluated against the volume itself, so item and quest conditions never pass) switch it off: while inactive it deals no damage and hides its mesh. It re-checks every tick because save restores replace flags silently. The Survey Launch's live water is one (20/s, off after `wreck.power_cut`). Note the firearm's object-type trace also hits these query-only overlap volumes (pre-existing).

Automation test `DeadCurrent.Combat.Health` covers damage, death, ignored extra hits, heal and reset.

The HUD shows `magazine | reserve` in the bottom right while a gun is drawn, `Reloading` during reload, and `[R] Reload` when the mag is empty and reserve remains.

Automation test `DeadCurrent.Combat.FirearmAmmo` covers magazine fill, consumption and partial reload.

## AI

`ADCScavengerCharacter` (`AI/`) is a mannequin pawn with health and an inventory. After death the corpse implements `IDCInteractable` (`Interaction.Loot`): **E** transfers the oldest stack into the player's inventory; repeat until empty. The prompt goes away when empty. Starting loot (12× 9mm, a field dressing, 2× salvaged wiring) is granted on first BeginPlay if the inventory was authored empty. `ADCScavengerController` runs a small state machine:

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

`UDCDialogueAsset` (`Dialogue/`) is a node graph Data Asset under `/Game/Dialogue`. Each node has a speaker, line, and choices. A choice's `NextNodeId` is empty to end the conversation. `Entries` pick the opening node: first entry whose conditions pass wins, otherwise `EntryNodeId`. Choices hide behind conditions and fire consequences, both from the shared rule language (see Rules). Consequences run before the next node shows. Digit keys **1–9** pick **visible** choices. Editor data validation flags missing entry/next nodes, duplicate ids, and nodes with no unconditional choice (the player could get stuck).

`UDCDialogueComponent` on the player runs the active conversation: `StartDialogue` / `SelectChoice` / `EndDialogue`. It evaluates everything through `FDCRuleContext::ForActor(owner)`. The canvas HUD draws the current line and numbered replies. Fire, inventory and interact are blocked while talking.

`DA_Dialogue_MaraIntro` is Mara's conversation, including all of Shore Watch (see First playable map).

## Rules: conditions and consequences

`Core/DCGameplayTypes.h` + `Core/DCGameplayRules.h` are the one rule language shared by dialogue, quests and inspectables (and later terminals and world events). Content authors lists of them; C++ evaluates them in one place.

| Condition | Passes when |
| --- | --- |
| `HasItem(Id, Quantity)` | the instigator carries at least Quantity of item Id |
| `QuestNotStarted(Id)` / `QuestActive(Id)` / `QuestComplete(Id)` | quest status (complete = any outcome stage) |
| `QuestStage(Id, Stage)` | quest is at that exact stage (use it to tell outcomes apart) |
| `WorldFlag(Id)` | world flag set |
| `ActorDead(Id)` | the actor registered under persistent id Id has a dead health component |
| `LocationDiscovered(Id)` | the location id has been discovered (see Exploration) |

Every condition has `bNegate`. A list passes when every entry passes; an empty list passes.

| Consequence | Does |
| --- | --- |
| `GiveItem` / `RemoveItem(Id or Item, Quantity)` | instigator's inventory |
| `StartQuest(Id, Stage?)` | start at Stage, or at the quest's start stage |
| `SetQuestStage(Id, Stage)` | move stage (to an outcome stage = complete) |
| `SetWorldFlag` / `ClearWorldFlag(Id)` | world state |

`FDCRuleContext` carries what rules read and write: instigator, its inventory and quest log, the world's `UDCWorldStateSubsystem` and `UDCPersistentRegistry`. Missing pieces fail conditions and skip consequences safely. `UDCGameplayRules::CheckConditionsFor` / `ApplyConsequencesFor` are the Blueprint entry points. `ValidateReferences` reports unknown quests, stages and items (used by `DeadCurrent.Content.Validate`).

**Adding a condition or consequence type** (skill check, reputation, companion present, discovered info): add the enum value and any field to `DCGameplayTypes.h` (use `EditCondition` so the editor shows only relevant fields), a case in `CheckCondition` / `ApplyConsequence` and in `ValidateReferences`, any new data the rule needs to `FDCRuleContext::ForActor`, and a test in `DCGameplayRulesTest.cpp`. Dialogue, quests and inspectables pick it up with no changes.

## World state

`UDCWorldStateSubsystem` (`World/`, world subsystem) owns named world flags (`shore.path_cleared`) and the `OnChanged` signal. Flags are set by consequences and read by conditions. `NotifyChanged` fires on flag changes and on any health-component death; quests re-check their transitions on it. Flags are saved and restored with the game. Name flags `<area>.<fact>`. The subsystem also holds `DiscoveredLocations`: `DiscoverLocation(Id)` returns true only the first time and fires `OnLocationDiscovered` and `OnChanged`; `IsLocationDiscovered(Id)`; `ReplaceDiscoveredLocations` (silent, used by load).

## Exploration

**Locations.** `ADCLocationVolume` (`World/`) is a box with `LocationId` and `DisplayName`. It **polls player positions at 4 Hz instead of using collision**, for two reasons: the firearm traces WorldStatic/WorldDynamic/Pawn, so a trigger box would stop bullets, and overlap events fire during the load teleport. It stops ticking once its location is discovered. First discovery shows a `LOCATION DISCOVERED / <name>` banner on the HUD, and the Tab journal has a PLACES list. Display names live on the volume actors; a future map screen or multi-map setup will want a location data asset.

**Containers.** `ADCLootContainer` (`World/`) is one reusable class: a mesh, a `UDCInventoryComponent` (the authored `Stacks` are the starting contents) and a persistent id. **E** takes the next stack. The prompt reads `Take 9mm Rounds (12) from Survey locker` and the message lists what is still inside; when empty it reads `Search X (empty)`. Persistence reuses the flat world-inventory arrays (see Save); there is no container-specific save code. A container missing from a save (added after it) keeps its authored contents.

**Flicker light.** `ADCFlickerLight` (`World/`) is a cosmetic point light plus a glow cube whose material `Color` parameter is scaled with the flicker. Random flicker and dropouts, and optional `ActiveConditions` (world conditions) to switch it off.

To add a location: place an `ADCLocationVolume` (or add a call in the map script), then use `LocationDiscovered(Id)` in any condition list.

## Quests

`UDCQuestDefinition` (`Quest/`) is a Data Asset under `/Game/Quests`: `QuestId`, `DisplayName`, `StartStage` (empty = first stage), and `Stages`. Each stage has:

- `StageId` and `ObjectiveText` (for an outcome stage, the journal summary of that outcome)
- `bCompletesQuest`: an outcome. A quest can have several; `QuestStage` conditions tell them apart
- `OnEnter` consequences, applied once when the stage is entered (never on save restore)
- `Transitions`: ordered `{Conditions, NextStage}`; the first whose conditions pass moves the quest on

`UDCQuestComponent` on the player is the quest log (`QuestId` → current stage, in start order): `StartQuest`, `SetStage`, `GetStage`, `GetQuestStatus`, `GetStageText`, `GetTrackedQuestId`. It re-checks transitions whenever its owner's inventory changes, the world state changes (flags, deaths), or a stage changes, chaining until nothing moves (loops are cut after 16 passes with a warning). So a quest can resolve by combat, by picking something up, or by any future condition type with no quest-specific code in actors. Starting a quest whose objective is already met jumps straight through. Editor data validation flags missing ids, bad start/next stages, and quests with no outcome.

Dialogue usually hands out rewards and moves return stages to outcomes; `OnEnter` sets the outcome's world flags so they hold however the stage was reached.

The HUD shows `<Quest>: <objective>` for the tracked quest at the top of the screen, and the Tab panel has a QUESTS journal (status, objective or outcome). The player shows a message when a quest changes stage or completes.

To add a quest: add a spec to `create_quest.py` (stages, transitions, OnEnter), add dialogue to `create_dialogue.py`, place any actors or inspectable variants in the map script, run `Tools\RebuildContent.bat`, then `Tools\RunTests.bat` (Content.Validate catches broken references).

## Inventory

`UDCInventoryComponent` (`Inventory/`) holds a list of `FDCItemStack` (definition + quantity). The player has one; corpses and containers use the same component, with starting contents set on the placed actor's `Stacks`.

- `AddItem(Item, Quantity)` tops up existing stacks, then starts new ones, never exceeding `MaxStackSize`. Returns the amount added. There is no capacity or weight limit yet.
- `RemoveItem(Item, Quantity)` takes from the newest stacks first and drops empty stacks. Returns the amount removed.
- `TransferAllTo(Destination)` moves every stack into another inventory (corpses, later containers).
- `GetQuantity(Item)`, `GetTotalWeight()`, `GetStacks()`, `IsEmpty()`
- `OnInventoryChanged` fires after every change (for UI, and later saving)

Saves will store each stack as `ItemId` + quantity.

Automation test `DeadCurrent.Inventory.Stacking` covers stacking and removal. Run tests with `Tools\RunTests.bat` (see Automated tests) or from the Session Frontend.

## Save

`UDCPersistentIdComponent` (`Save/`) holds an authored `FName` unique within the map (e.g. `gym.scavenger`, `gym.mara`). `UDCPersistentRegistry` is a world subsystem that registers those IDs at BeginPlay and can `FindActor` by id. Duplicate ids log an error and are rejected.

`IDCPersistent` is the save hook: `GetPersistentId`, `CapturePersistentState`, `ApplyPersistentState`. The scavenger, Mara, the gym door and world pickups implement it. Capture stores id, existence, alive/dead, door yaw, and inventory stacks. Cyan debug labels can draw the id above those actors (`bDrawId`, off by default). Scavenger patrol/chase text (`bDrawState`) is also off by default.

Gym IDs: `gym.scavenger`, `gym.mara`, `gym.door`, `gym.pickup_pistol`, `gym.pickup_ammo`, `gym.pickup_dressing`, `gym.pickup_wiring`, `gym.pickup_coil`.

Boathouse IDs: `boat.scavenger`, `boat.mara`, `boat.door`, `boat.pickup_pistol`, `boat.pickup_ammo`, `boat.pickup_dressing`, `boat.pickup_coil`, `boat.wreck_locker`, `boat.wreck_tender`.

`UDCSaveGame` is the slot (`DeadCurrent`, user 0; tests switch to a scratch slot with `UDCSaveSubsystem::SetSlotName`). `UDCSaveSubsystem` (`UGameInstanceSubsystem`) writes player transform, health, inventory, equipped magazine, the quest log (quest id + stage), world flags (from `UDCWorldStateSubsystem`), and every registered persistent actor. World-actor inventories are stored as parallel primitive arrays (`WorldInvActorIds` / `WorldInvItemIds` / quantities / paths) because nested `TArray` stacks inside `WorldActors` or `ActorInventories` can serialize empty through `USaveGame`.

F5 saves. Saving while dead is refused ("You can't save now."). F9 reads the slot, **reopens the saved map** (`MapPackage`, else `MapName`), and `ADCGameMode::StartPlay` calls `ApplyPendingLoad` once every actor has begun play. Loading therefore always starts from a fresh map (taken pickups come back, dead enemies are alive) and then applies the save, so a load inside a running session behaves exactly like a load after relaunching, loading while dead or mid-dialogue is clean, and loading an earlier save can never lose an item the world already destroyed. Apply order: world flags and discovered locations first (silently, so discovery volumes and world-conditioned hazards already see them and nothing is re-announced), then world actors (destroy pickups missing from the save, restore deaths, which marks the health component dead so corpses stay lootable and `ActorDead` holds; then corpse and container inventory from the flat arrays), then the player (transform, health, inventory without a magazine refill), then the quest log. Restoring never re-runs stage `OnEnter` consequences.

Save format version (`UDCSaveGame::SaveVersion`, current 3): 0/1 = first playable; 2 adds `MapPackage`, world flags owned by the world-state subsystem, and data-driven quest stages; 3 adds `DiscoveredLocations`. Older saves load with no discoveries.

**Known latent issue:** "missing from the save" means "taken", so loading a save made before an `ADCItemPickup` was added to the map destroys that pickup. The Survey Launch uses only containers (which keep their contents when missing from a save), so nothing is affected today, but fix it (track removed ids explicitly) before content adds pickups. Older saves still load (map reopened by short name); a saved quest stage that no longer exists in the quest data is dropped with a warning so the quest can be taken again. `MapName`/`MapPackage` route the load to the right map; there is still only one production map.

Automation tests `DeadCurrent.Save.PersistentId`, `DeadCurrent.Save.InventoryRestore`, and `DeadCurrent.Save.WorldInventorySlot` cover lookup, inventory snapshot restore, and USaveGame round-trip of corpse loot. `DeadCurrent.Quest.Persistence` covers the quest log and flags, and `DeadCurrent.Map.Boathouse.*` covers full F9 loads in the real map.

`ADCHUD` is a temporary canvas HUD: crosshair dot, interaction prompt, timed messages via `ADCHUD::ShowMessageFor`, the inventory panel with the quest journal beside it, the weapon ammo readout, a health bar, the dialogue panel, and the tracked quest objective. It will be replaced by UMG widgets when the HUD grows.

## C++ vs Blueprint / data

- **C++**: foundational systems (interaction, inventory runtime, combat and damage, quests, dialogue runtime, save and world state, AI interfaces, gameplay rules).
- **Blueprints and Data Assets**: content (weapons, NPCs, encounters, quests, dialogue, items, perks, locations, effects).
- A designer should be able to add content without touching foundational C++.
- C++ classes are `abstract` bases when content is expected to subclass them in Blueprint.

## Source layout

Single runtime module `DeadCurrent`. The module root is a public include path, so includes are written as `"Folder/File.h"`.

| Folder | Owns |
| --- | --- |
| `Core/` | Game mode, Gameplay Tag declarations, the shared rule language (conditions/consequences), content loading, test helpers |
| `Character/` | Player character, player controller, camera manager |
| `Interaction/` | Interaction interface, detection, prompts |
| `Items/` | Item definitions |
| `Inventory/` | Inventory runtime |
| `Combat/` | Weapons, damage processing, health |
| `AI/` | Enemy controllers, perception, behavior |
| `Dialogue/` | Dialogue data and runtime |
| `Quest/` | Quest definitions and the player quest log (conditions/consequences live in `Core/`) |
| `Save/` | Save game, persistent IDs, persistence interfaces |
| `UI/` | HUD and widget base classes |
| `World/` | Persistent world objects, inspectables, and `UDCWorldStateSubsystem` (world flags) |

Split into more modules only when a boundary is proven (for example an editor-only tools module).

## Content layout

| Folder | Owns |
| --- | --- |
| `Characters/` | Character meshes and animation. `Mannequins/` comes from the UE template packs. |
| `Dialogue/` | Dialogue assets |
| `Quests/` | Quest definition assets |
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
| `Item.` | Item classification (`Item.Weapon.Firearm`, `Item.Ammo`, `Item.Quest`) |
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
