# DEAD CURRENT - Technical Architecture

Living document. Update it whenever a foundational system lands or a convention changes. Art, audio, the external asset library, and Meshy generation are in `Design/art_pipeline.md`.

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

Micro RPG (LongTermPlan Phase 2) is accepted. Shore Watch was played through during the Exploration Loop playtest with no issues: a combat route and a coil (stealth) route, different outcomes, and save/load at every stage. Exploration Loop (LongTermPlan Phase 3) is accepted after human playtest (2026-09-28): the Wrecked Survey Launch, an optional point of interest west of the boathouse (location discovery, environmental clues, live-water hazard, two loot containers, one line from Mara). See `ExplorationLoopPlan.txt` for the scope. RPG Layer (LongTermPlan Phase 4) is accepted after human playtest (2026-09-29): Engineering, Survival, and Persuasion builds each open a different reading or line on that same shore, and the build survives save/load. See `RPGPhasePlan.txt`. The Presentation Pass (art, materials, water, bodies, first audio, no gameplay change) is accepted after two human playtests (2026-09-29). See `PresentationPassPlan.txt` and the Presentation layer section. World State (LongTermPlan Phase 5) is accepted (2026-09-30): conditional presence and the Landing Stage, whose look and Mara's place follow the Shore Watch outcome, plus acceptance-playtest changes to Shore Watch (a sneak route behind a windbreak, a scavenger who warns first). See `WorldStatePhasePlan.txt`. Phase 6, the Vertical Slice, started on 2026-09-30 (plan approved; `VerticalSlicePhasePlan.txt`). It adds a second production map, `Lvl_PointeSombre`; the tooling and test infrastructure changes for it are recorded below as they land.

## Automated tests

```
Tools\RunTests.bat              every test: editor-context suite, then the in-map suite
Tools\RunTests.bat Quest        only DeadCurrent.Quest.*
Tools\RunTests.bat Map          only the in-map suite, on every production map
Tools\RunTests.bat Map.Sombre   one map's group (DeadCurrent.Map.<Group>.*), on the map registered for it
Tools\RunTests.bat -nomap       skip the in-map suite
Tools\RunTests.bat -build       build DeadCurrentEditor first
```

Logs go to `Saved/Logs/RunTests.log` and `RunTests_Map.log` (`RunTests_Map_<Group>.log` for other maps). Each log is deleted before its run, and a run that writes none fails, so a failed engine start cannot read as an old pass. A registered production map that is not built fails `RunTests.bat` and `Package.bat` instead of being skipped (VS-04). The full run takes a few minutes including editor start-up. Count is recorded in `Design/CLAUDE_SESSION_REPORT.md`.

| Group | Covers |
| --- | --- |
| `DeadCurrent.Progression.*` | Attribute, skill and perk lookup, point caps, effective skill, the three build conditions, check labels, a hidden dialogue choice, inspect variants, save/load and a pre-RPG save |
| `DeadCurrent.Rules.*` | Every condition and consequence type, lists, empty contexts, reference validation |
| `DeadCurrent.Quest.*` | Stages, start stage, outcomes, event-driven transitions (death, item), branch order, one quest moving another, save/restore of progress, stale stages |
| `DeadCurrent.Dialogue.*` | Graph walking, conditional entries, hidden choices, choice consequences (quest, items, flags) |
| `DeadCurrent.Content.Validate` | Every quest and dialogue asset: graph checks and references to real quests, stages and items; Asset Manager registration |
| `DeadCurrent.Content.ShoreWatch.*` | The shipped quest and dialogue assets through both routes, pre-quest shortcuts, the clue line and epilogues, with save/restore at each stage |
| `DeadCurrent.Content.Sombre.*` (VS-09) | The slice's data without the map. `Items`: the ten slice items resolve as `Item.Quest`, and the shipped loot is reused, not reminted. `QuestGraph`: the ledger's stage ids; `knows` needs both the panel and the note; the storm cleared at `knows` and set again by every `done_*`; every outcome reachable, untouched included; chaining from flags set before Varga; False Light by both approaches. `Dialogue`: the nine assets' entry lists against the ledger; an unconditional choice on every node; only ledger flags; no tie leak; evidence needs and removes its item; exclusive keepers; Varga's `first` starts both quests; Marthe's call; Dell's §8.2 wording; a zero-investment run |
| `DeadCurrent.Exploration.Discovery` | Once-only location discovery, rotated volume, announce count, save round-trip, silent restore, pre-Phase-3 save |
| `DeadCurrent.Exploration.Container` | Prompt text, partial and full looting, slot round-trip into a fresh world, a container newer than the save |
| `DeadCurrent.Exploration.WorldConditions` | Damage volume and flicker light switched by a world flag, including silent restore |
| `DeadCurrent.Content.Exploration.MaraWreckLine` | The shipped dialogue: Mara's wreck exchange across Shore Watch states, offered once, survives save/reload |
| `DeadCurrent.Map.Boathouse.*` | Game context, real `Lvl_Boathouse`: placed actors, real interactions and damage, real F9 loads that reopen the map (pre-quest, ready to turn in, complete, legacy save). `SurveyLaunch`: walk-in discovery, live-water damage, clues, partial loot, pull the leads, hidden kit, save, diverge, F9, no re-announce. `SurveyLaunchSaves`: the POI combined with Shore Watch accepted or complete. `BuildChecks`: the same actors change with Engineering, Survival, Fieldcraft, Persuasion, the three perks and the Sounder Chart, then a real F9 restores the build. `LandingStage` (Phase 5): the stage and Mara through both Shore Watch routes, deferral while the player is with Mara, save, diverge, F9 snaps back, the power cut, the tackle box, the dressing. `LandingStageSaves`: hand-written version-5 saves show the derived state, undiscovered. `LandingStagePlayerSave`: the player's own save, copied to a scratch slot. `CampCover`: three crate stacks outside the patrol square, one breaking his sight line to a crouched player near the coil. Scratch save slots |
| `DeadCurrent.Map.Boathouse.TestHelpers` | (VS-02) The shared in-map harness on the real map: find by id and display name, talk and pick replies, teleport, F5, diverge, F9, and `QueueWaitUntil` |
| `DeadCurrent.World.ConditionalPresence` | `ADCConditionalPresence`: default, first match, move with offsets, hide and re-show, deferral while observed, restore snaps, destroyed target, writes nothing |
| `DeadCurrent.World.CellPortal` | (VS-03) `ADCCellPortal`: locked text and prompts, first-match variants, consequences once per use, arrival at the destination at rest, the scene cut snapping deferred presence (and never standing in for a restore), the timed transition, no destination refused, saves nothing; (VS-04) every portal refuses while any transition runs |
| `DeadCurrent.Map.Sombre.*` | (VS-04) Game context, real `Lvl_PointeSombre`: `Architecture`, `CrossMapLoad`, `Respawn`, `Atmosphere`; (VS-06) `BiomeExclusions`; (VS-08) `Greybox`; (VS-10) `Crossing`. See "Pointe Sombre: map architecture" |
| `DeadCurrent.AI.ScavengerNotice` | The scavenger's notice rule: standing, the sight sense decides; crouched, only within 8 m and 45° |
| `DeadCurrent.Map.Boathouse.ScavengerWarning` | Spotted from the path he warns, backing off returns him to his loop, he warns again, and coming within 3 m starts the chase |
| `DeadCurrent.Presentation.ConditionalAudio` | `ADCConditionalAudio` follows a world flag (hum until cut, one-shot on the rising edge, silent when already set at start, re-arms after a silent restore) and writes no world state |
| `DeadCurrent.World.InspectVariants`, `.Save.*`, `.Inventory.*`, `.Combat.*` | Inspectable variants (including per-variant verbs) and the first-playable systems |

`Core/DCTestHelpers.h` has `FDCTestWorld`, a throwaway game world with subsystems and BeginPlay, for tests that need a registry, world state or component events.

`Core/DCMapTestHelpers.h` (namespace `DCMapTest`, Phase 6 VS-02, extracted from `DCBoathouseMapTest.cpp`) is the shared harness for in-map tests: `GameWorld`, `Player`, `Find(persistent id)`, `Saves`, `WorldState`, `Inspectable(display name)`, `Nearest<T>`, `Use`, `Teleport`, `TalkTo(persistent id)`, `Say(reply text)`, `VisibleChoiceTexts`, `Count(item id)`, `Stage(quest id)`, `Health`, `HUD`/`Message`/`Banner`, `SwitchSlot`, and the latent steps `QueueFreshMap(map, scratch slot)`, `QueueLoad` (F9 and wait for the reopened map), `QueueCleanup(scratch slot)`, and `QueueWaitUntil(condition)`. Nothing in it is map-specific: the map path, the scratch slot, ids, and coordinates are arguments, so a new map's test file starts from it (the header's comment has the skeleton) and keeps its own coordinates and lookups. One scratch slot per test file. `DCBoathouseMapTest.cpp` uses it through `using` declarations and thin wrappers; `DCLandingStageMapTest.cpp` still carries its own copies of the same helpers (an accepted file left unchanged on purpose). `DeadCurrent.Map.Boathouse.TestHelpers` proves the harness on the real map and is the smallest worked example.

In-map tests (`EAutomationTestFlags::ClientContext` only) run under `-game`; with rendering enabled the coil-route test also saves `Saved/Screenshots/<platform>/DC_QuestHUD.png`. `DeadCurrent.Review.Capture` is also client-context, and it is not part of this run. See Review capture.

`Tools/PlayTest.bat` launches the game standalone (no editor) in a 1280x720 window on the discrete GPU. It forces DX11, Low scalability, 70% resolution, no Lumen / ray tracing / virtual shadows / volumetric clouds / fog / SSAO / bloom / motion blur, a 400 MB texture pool, no vsync, and a 60 FPS cap. `Tools\PlayTest.bat <map>` opens a specific production map (`Lvl_PointeSombre` or `Sombre` once it exists); with no argument it uses the project's default map. Those overrides apply from the first frame (`-dpcvars`); the project's own rendering settings are unchanged. It uses the compiled editor build, so rebuild `DeadCurrentEditor` after C++ changes. The first launch (and the first launch after switching graphics APIs) compiles shaders and takes several minutes.

## Review capture

`Tools\ReviewCapture.bat` is a tool, not a pass/fail gate, and `Tools\RunTests.bat` does not run it. Close the editor first. It launches the same 1280×720 DX11 window and the same `-dpcvars` as `Tools\PlayTest.bat`, then runs `DeadCurrent.Review.Capture` on the map given as its first argument (`Tools\ReviewCapture.bat [map]`; `Lvl_Boathouse` by default, `Lvl_PointeSombre` or its alias once that map exists), with the art layer loaded. The batch passes `-ReviewMap=/Game/Maps/<map>`; the test has no map hard-coded.

Viewpoints, the walking route, and each shot's expectation live in JSON, not in C++: `Tools/Review/<map>.json` plus every `Tools/Review/<map>/*.json` (one file per content cell, each owned by one builder; `Lvl_Boathouse` has only its single file). The set is merged in file-name order: viewpoint ids are unique across it, exactly one file defines the `route`, and an optional top-level `"art_sentinels"` (default 1; 0 to not wait) says how many `ArtLayerSentinel` actors mark the art sublevels as loaded. An expectation is one or two sentences on what the shot should show and what should read first, with a citation. For each viewpoint the test applies the optional setup through the world-state subsystem, the quest log, and the progression component, places the player camera, lets the frame settle, and writes a PNG. A viewpoint with `subject` or `subjects` aims at that actor's bounds center from `view_offset` (`local` is the actor's rotation; `world` is used for a group), then steps outward until the eye is clear of static geometry. It then walks the real path from the boathouse door, past the scavenger camp, to the Survey Launch beach, and writes a frame about once a second. Discovery banners and timed messages are cleared before every shot except `hud_inspect`, which keeps its inspect line.

The folder is `Saved/Review/<yyyy-mm-dd_hhmm>/` (`<yyyy-mm-dd_hhmm>_<map>/` for any map other than `Lvl_Boathouse`): the PNGs, `contact_sheet.png`, and `manifest.json` (UTF-8, no BOM: the map, the view files it used, then for each viewpoint its id, setup, expectation, source, image file, camera, frame time, and check results). The checks, for each viewpoint, are frame time, any visible primitive still on an engine default or grid material or with a missing texture, texture-pool over-budget warnings, Warning and Error log lines from that capture, and, for the named landmarks, whether a visibility trace from the camera reaches a point on the landmark's visible surface (the landmark and anything containing that point are ignored) and whether that point is in the view frustum. Each viewpoint's checks also carry cost measurements for same-session A/B gates (Phase 6): `visible_primitive_components`, `visible_material_slots`, and `visible_instances` (instances in visible instanced components) from the scene scan, and `rhi_draw_calls_peak` and `rhi_primitives_peak`, the highest per-frame RHI counts sampled while the frame settled (the module dependency is `RHI`). Those results stay in the manifest. The test fails only when it could not capture. A route may name the ground it walks on (`"ground_tag"`; Pointe Sombre: `SombreTerrain`): each frame is then placed by a trace from `probe_from_cm` (default 200 m) straight down, ignoring the player, and the capture fails if the first hit is not that terrain (VS-06: the old 4 m probe started inside the tower rock and shot from inside the island, so gate 0's route figure is not a walk on the surface). For same-session A/B gates, set `DC_REVIEW_ARGS` (and `DC_REVIEW_LABEL`, appended to the folder name) before `ReviewCapture.bat`: `-ReviewMaxFPS=0` lifts the 60 FPS cap, `-ReviewSampleSeconds=<s>` samples each view longer, `-ReviewNoRoute` skips the route, `-ReviewHideTag=<tag>[+<tag>]` hides tagged actors or single tagged components, and `-ReviewToggleTag=<tag>` with `-ReviewToggleViews=<id prefix>` and `-ReviewToggleCycles=<n>` hides and shows the tag at each matching view in ABBA windows and records each state's median frame time (`toggle` in the view's record; with no `-ReviewToggleViews`, every view, a VS-08 fix: `FString::StartsWith` is false for an empty prefix, so the first gate-1 run toggled nothing). The manifest records which of these were used. Pointe Sombre's storm look fires random lightning (`Lightning:<look>`): a frame can land on a flash and render the ground white, so review sets meant for judging pass `-ReviewHideTag=Lightning`. It uses the scratch save slot `DeadCurrent_Review` and deletes it when it finishes.

## Editor scripts

`Tools/EditorScripts/` holds Python scripts that create or regenerate assets, so generated content can be rebuilt instead of hand-edited. They need the editor-only `PythonScriptPlugin` and `EditorScriptingUtilities` plugins, which the project enables. Run one headless with:

```
"C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "<repo>\DeadCurrent.uproject" -run=pythonscript -script="<repo>\Tools\EditorScripts\<script>.py" -unattended -nullrhi
```

| Script | Does |
| --- | --- |
| `setup_player_input.py` | Creates the player input actions (sprint, crouch, interact, inventory, fire, reload, holster), maps them in `IMC_Default`, assigns them on `BP_FirstPersonCharacter`. Safe to re-run; add new player actions here. |
| `create_items.py` | Creates or updates the item definitions in `/Game/Items` from every `Tools/ContentSpecs/items/*.py`. Safe to re-run; edits made in the editor to those items are overwritten. |
| `create_dialogue.py` | Creates or updates dialogue Data Assets in `/Game/Dialogue` from every `Tools/ContentSpecs/dialogue/*.py`. Safe to re-run. |
| `create_quest.py` | Creates or updates quest Data Assets in `/Game/Quests` from every `Tools/ContentSpecs/quests/*.py`. Safe to re-run. |
| `content_specs.py` | (VS-02) The spec loader and the `cond` / `cons` helpers the three `create_*.py` scripts share. Not run on its own. |
| `dump_content.py` | (VS-02) Read-only dump of every generated item, quest, and dialogue asset's properties to JSON (`Tools\DumpContent.bat <label>` writes `Saved/ContentDumps/<label>.json`). An empty diff between two dumps proves a generator change kept the content. |
| `build_test_gym.py` | Regenerates `/Game/Maps/Lvl_TestGym`. Hand edits to that map are lost on the next run. |
| `import_art.py` | Imports CC0 shore surfaces into `/Game/Art/<Source>/<AssetId>/` at 2K and creates `M_DC_Surface` plus the `MI_DC_*` surface instances, the *Tern* wreck master (`M_DC_Wreck`: photo diffuse, tint, rust grime, then desaturated and cast cool so it reads as faded paint), the lake master `M_DC_Lake` and its instances (dark, two slow world-space normal layers, default-lit, not the Water plugin), the Meshy prop meshes, the name-board letters and chalk textures, and the Survival_Character skeleton compatibility and costume tints. Safe to re-run; existing textures are replaced in place. |
| `import_audio.py` | Imports the game-ready CC0 sounds from `C:\FO5_AssetLibrary\Audio` into `/Game/Audio/{Ambience,SFX,Weapons}` (loop flag on the beds and hums) and creates the `SA_DC_Hum`, `SA_DC_Snap`, and `SA_DC_Clunk` attenuation assets. Safe to re-run. |
| `build_boathouse.py` | Regenerates the persistent `/Game/Maps/Lvl_Boathouse` (Shore Watch and the Survey Launch). Also creates the material instances in `/Game/Environment/Materials` (`MI_DC_*`, children of `M_FlatCol` and our own `M_DC_Glow`, an unlit translucent glow with `Color` rgb = emissive, a = opacity). Hand edits to that persistent map are lost on the next run. It re-links the streaming sublevel `/Game/Maps/Lvl_Boathouse_Art` and does not edit or save it. Requires the surface instances from `import_art.py`. |
| `dress_shore.py` | Imports three CC0 driftwood meshes and replaces actors tagged `ShoreDress` in `Lvl_Boathouse_Art`. NoCollision. Safe to re-run. Not part of `RebuildContent.bat`; a content rebuild leaves those actors. |
| `dress_structures.py` | Imports the *Tern* and the CC0 crate, lamp, and can, and replaces actors tagged `StructureDress` in the art level. NoCollision. The tender mesh is assigned by `build_boathouse.py` on the existing container. |
| `build_landing_stage.py` | The Landing Stage POI (Phase 5). Not run on its own: `build_boathouse.py` calls `build(<its helpers>)` near the end of `main()`. Owns only the actors in folder `LandingStage`. |
| `dress_landing.py` | Places the Landing Stage's NoCollision dressing (tag `LandingDress`) in `Lvl_Boathouse_Art`. Not part of `RebuildContent.bat`; a content rebuild leaves them. |
| `export_pack_textures.py`, `reimport_pack_textures.py` | Helpers for `Tools\ImportPackAssets.ps1` (generic pack migration with textures cut to 1K). |
| `build_pointe_sombre.py` | (VS-04) Regenerates `/Game/Maps/Lvl_PointeSombre`: the core script, then one `build(tk)` hook per cell in `pointe_sombre/`. See "Pointe Sombre: map architecture". |
| `biome/scatter.py`, `biome/plan.py`, `biome/verify_plan.py`, `biome/prepare_biome_meshes.py` | (VS-06) The rocky-shoreline recipe: writes `Lvl_PointeSombre_Biome` and its manifest. See "Shoreline recipe". |
| `dress_audio.py` | Places the always-on shore wind and lap beds (two 2D `ADCConditionalAudio`, tagged `ShoreAudio`) in `Lvl_Boathouse_Art`. Not part of `RebuildContent.bat`; a content rebuild leaves them. |
| `inspect_template.py` | Read-only dump of player movement settings, input mappings and level actors. |

`Tools\RebuildContent.bat` runs `import_art`, `import_audio`, `create_items`, `create_quest`, `create_dialogue`, `build_test_gym`, `build_boathouse`, `build_pointe_sombre`, and `biome\scatter` (VS-06) in that order (the pistol references the audio; dialogue references items; maps reference everything). `Tools\RebuildContent.bat create_quest` runs one; a script in a subfolder of `Tools/EditorScripts` is named with its folder (`kit\build_kit_gym`). `Tools\ImportSurvivalCharacter.ps1` is a one-time pack migration, not part of the rebuild (see `art_pipeline.md`). `Tools\Package.bat` cooks a Development Win64 build to `Saved\Packaged` and smoke-launches every production map. Close the editor first and rebuild C++ before running it. Logs: `Saved/Logs/RebuildContent_<script>.log`. A script fails the run on a Traceback or on the commandlet's "Python script executed with errors" (a syntax error prints no Traceback), and a one-script run exits with its result (VS-04 fixed both: before, a syntax error and a failed one-script run both reported success).

## Content specs (Phase 6, VS-02)

Items, quests, and dialogue are data in **one file per owner** under `Tools/ContentSpecs/{items,quests,dialogue}/`. The three `create_*.py` generators hold no content: they load every spec file in their folder (file-name order; `_`-prefixed files are skipped), check that asset names and ids are unique across files (a clash stops the run and names both files), and write the Data Assets. A spec file defines one list (`ITEMS`, `QUESTS`, or `DIALOGUES`) and is run with `cond`, `cons`, `COND`, `CONS`, and `unreal` already defined (`content_specs.py`); `cons("GIVE_ITEM", id=...)` resolves the asset reference from the item's own spec, so a dialogue can give an item defined in another file. The accepted Shore content lives in `items/shore.py`, `quests/shore_watch.py`, and `dialogue/mara_intro.py`, moved unchanged: `Tools\DumpContent.bat` before and after the move gave byte-identical dumps, and regeneration left every `.uasset` byte-identical. The contract, the rules, and how to change a generator safely are in `Tools/ContentSpecs/README.md`.

**The Pointe Sombre slice's data (VS-09, WP-NARR)** is in the same folders, with ids from the ledger `Design/POIs/sombre_ids.md`:
- `items/sombre.py`: ten `Item.Quest` items (`DA_Item_LivLetter` … `DA_Item_SombreVaultKey`)
- `quests/sombre_characteristic.py` and `quests/sombre_false_light.py` (`DA_Quest_SombreCharacteristic`, `DA_Quest_SombreFalseLight`)
- nine `dialogue/sombre_<name>.py` files (`DA_Dialogue_Sombre<Name>`)

A node's `speaker` key names who says the line; the meeting's reactions are spoken inside Marthe's asset. The cells place the assets: their NPCs, inspectables, and portals set the flags these specs read. The changes from the dialogue document are listed in `Design/Narrative/SLICE_DIALOGUE.md` ("Applied in VS-09"). Adding the slice left every Shore asset byte-identical (`DumpContent` before and after).

## Player controls

| Action | Keyboard / mouse | Gamepad |
| --- | --- | --- |
| Move | WASD, arrow keys | Left stick |
| Look | Mouse | Right stick |
| Jump | Space | A / Cross |
| Sprint (hold, forward only) | Left Shift | Left stick click |
| Crouch (toggle) | Left Ctrl, C | B / Circle |
| Interact | E | X / Square |
| Inventory and character sheet (toggle) | Tab, I | View / Back |
| Build panel (toggle) | B | |
| Raise attribute / skill / take perk / reset | F1–F3 / F4–F6 / F7–F9 / F10, while the build panel is open | |
| Fire | Left mouse | Right trigger |
| Reload | R | Y / Triangle |
| Holster / draw (not while talking) | 1 | D-pad up |
| Dialogue reply | 1-9 | |
| Save / load | F5 / F9 | |

While the build panel is open, F5 and F9 allocate Survival and Relay Ear. They do not save or load until the panel is closed.

The engine debug view modes on F1–F5, and the F9 screenshot toggle, are removed in `Config/DefaultInput.ini`. Those keys stay with the build panel, save, and load. Restart the editor or `PlayTest.bat` after that config change. A package cooked before it still has the old bindings.

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

`Lvl_Boathouse` is the first-playable scenario (not World Partition). The persistent map is generated by `build_boathouse.py`. Its light is a low-key overcast lake: a dim cool sun, a heavy grey sky, non-volumetric height fog, a darker open-lake flat, and a local fill inside the boathouse. That grade does not use volumetric fog, bloom, or Lumen, so it still applies under `Tools\PlayTest.bat`. Dressing that is not a gameplay actor lives in the always-loaded streaming sublevel `Lvl_Boathouse_Art`, which the script re-links and does not rewrite. A sentinel tagged `ArtLayerSentinel` marks that level; `DeadCurrent.Map.Boathouse.ArtLayer` checks it is loaded. The player wakes inside the boathouse, takes the pistol from the workbench, goes out the door, meets a scavenger on the shore path, then finds Mara behind a ridge out of the scavenger's sight. Persistent IDs use the `boat.` prefix.

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
- **Clues** (all `ADCInspectableActor` with variants): name board, life jackets, depth sounder, survey log (verb "Read", sets `wreck.log_read`), breaker panel (changes after the log is read; Engineering 2 adds a reading about the shore-power breaker and the mast lamp), battery bank (first inspect sets `wreck.battery_seen`; the verb then becomes "Pull the leads", which sets `wreck.power_cut`), emergency beacon (reacts to `shore.relay_inspected` and to the power being cut), dead fish (Pulse Read adds a one-shock reading), chalk warning (Fieldcraft 2 reads who wrote it), and the west window (changes after discovery and after the power is cut). Survival 2 changes the life jackets. The depth sounder changes when the player carries `survey_chart` and has Engineering 2 or Schematic Eye.
- **Hazard:** the water is a permanent dark, no-collision `WaterSurface` slab. Over it, `LiveWater` is an `ADCDamageVolume` (X -2020..-980, Y -1780..-1000, 20/s, `ActiveConditions = [!wreck.power_cut]`) whose bright blue glow mesh is the "electric" layer, with six flickering spark lights. A ring of pale dead fish lies on the surface at its edge, and a chalked warning plank (`Chalk warning`) stands on the beach at the edge. Pulling the leads switches the damage, the glow and the sparks off and persists through saves; the water itself stays (playtest feedback: it must not vanish).
- **Loot** (`ADCLootContainer`): `boat.wreck_locker` (survey locker in the wheelhouse: 12× 9mm, 3× Salvaged Wiring, 1× Field Dressing) and `boat.wreck_tender` (a small boat about 4 m off the stern, inside the live water, tied to the transom by a line: 1× Sounder Chart, 2× Field Dressing, 18× 9mm; the chart is the site's novel item). The log mentions the tender.
- **Mara:** once `wreck.log_read` is set, her greeting, who, place, in-progress and epilogue nodes (never the turn-ins) offer "There's a wrecked survey launch west of the boathouse. I read her log." She answers; the player can ask "Was it a storm?" (sets `wreck.mara_told`). Offered once. On that storm line, Persuasion 2 (effective) adds "You're leaving something out." (sets `wreck.mara_pressed`). PROVISIONAL: she saw two people leave the beach and did not follow.
- **World state ids:** location `shore.survey_launch`; flags `wreck.log_read`, `wreck.battery_seen`, `wreck.power_cut`, `wreck.mara_told`, `wreck.mara_pressed`. Never rename a shipped id.
- **Relay Ear:** after `shore.relay_inspected` is set (the first look still sets it for everyone), the perk adds a pinout reading. It does not skip the Shore Watch clue.

The POI adds no quest and no wreck-specific C++. Build checks are shared conditions on the existing actors. Pulling the leads, both containers, and both Shore Watch routes still work with nothing invested.

### Point of interest: the Landing Stage (Phase 5, World State)

Spec: `Design/POIs/shore.landing_stage.md`. A timber landing stage in the shallows just east of the boathouse door, south of the path (gangway X 950..1070 from the curb; deck X 850..1270, Y −820..−1120, top Z 30), with a lean-to over a crate, a storm lantern on a post, a string of bare bulbs, a skiff moored along the west side, and a tackle box. It is built by `Tools/EditorScripts/build_landing_stage.py`, which owns only the stage; `build_boathouse.py` calls it through one hook (`build_landing_stage_poi`) that hands it the map script's helpers. Its actors are in the persistent map (outliner folder `LandingStage`, tag `LandingStage`) because presence rules reference them. The walkable deck and gangway are hidden collision blocks under NoCollision plank meshes from the migrated `Smugglers_cove` pier kit. Static dressing is in the art level (`dress_landing.py`, tag `LandingDress`).

The stage reads the Shore Watch outcome from existing flags only, through eight `ADCConditionalPresence` rules (tagged `Presence_<Name>`), combat listed first so that a kill followed by a coil hand-over stays the combat picture:

| State | Flag | Stage | Mara |
| --- | --- | --- | --- |
| Combat | `shore.path_cleared` | lantern light, card, and mooring line hidden; crate lid thrown on the deck; contents hidden; cut line shown; skiff moved out to (1720, −2080) | stays at the lookout, with her pack |
| Coil | `shore.relay_recovered` | lid shut; lashing and skiff cargo shown; contents hidden | moved to the deck (960, −990), her pack beside the crate |
| Default | neither | lantern lit, lid leaning, contents shown, skiff moored | at the lookout |

`wreck.power_cut` turns the bulbs (`ADCFlickerLight`, `ActiveConditions`) and their hum (`ADCConditionalAudio`) off. Inspectables with variants: `Crate`, `Storm lantern`, `Card`, `Bulbs`, `Skiff`, `Cut line`. Discovery: `ADCLocationVolume` `shore.landing_stage` / "Landing Stage". Container: `landing.tackle` (6× 9mm, 1× Salvaged Wiring). No new flag, quest, item, dialogue, or save field. Tests: `DeadCurrent.Map.Boathouse.LandingStage`, `DeadCurrent.Map.Boathouse.LandingStageSaves`.

## Pointe Sombre: map architecture (Phase 6, VS-04)

`/Game/Maps/Lvl_PointeSombre` is the slice's map (`VerticalSlicePhasePlan.txt` §5). It is one always-loaded persistent map (not World Partition): an exterior island plus interior cells built far away in the same map, reached through cell portals. Like `Lvl_Boathouse` it is generated: hand edits are lost on the next `Tools\RebuildContent.bat build_pointe_sombre` (also part of a full rebuild). The generated `.umap`, the terrain tiles, and `Tools/PointeSombre/out/terrain_probe.json` are build products the Integrator commits.

### The scripts and who owns them

| File | Does | Owner |
| --- | --- | --- |
| `Tools/PointeSombre/island.json` | The map frame and terrain as data (metres): coast outline, shore steepness zones, hills, ridges, flattened pads, the reef causeway and reef, noise seed, the walkable outline, reference places | Integrator |
| `Tools/EditorScripts/pointe_sombre/island.py` | Pure Python (no engine): the deterministic heightfield and outline tests. The map script, the route-timing report, and the biome tool all read the same island from it | Integrator |
| `Tools/EditorScripts/pointe_sombre/terrain_mesh.py` | The terrain tiles (below) | Integrator |
| `Tools/EditorScripts/pointe_sombre/toolkit.py` | The helpers every cell script gets as `tk`: ownership (`tk.own`), boxes and walls, `tk.portal` / `tk.portal_variant`, `tk.presence` / `tk.state` / `tk.placement`, `tk.anchor`, `tk.interior_origin`, `tk.make_interior`, `tk.marker`, `tk.ground_z`, lights, post-process boxes, location volumes, materials (`tk.surface`, `tk.tinted_surface`, `tk.flat`, `tk.glow`). VS-10 added the story actors: `tk.inspectable` / `tk.variant`, `tk.npc` (pack body with `tk.costume` tints, or the Quinn placeholder with flat paints; dialogue asset; persistent id), `tk.flicker_light`, `tk.conditional_audio`, `tk.set_persistent_id`. `tk.cons("GIVE_ITEM"/"REMOVE_ITEM", id=...)` sets the item reference from the content specs | Integrator |
| `Tools/EditorScripts/build_pointe_sombre.py` | The core: terrain, water, walls, nav bounds, the three atmospheres, the exterior ambience, anchors, the player start and its respawn rule, then one `build(tk)` hook per cell (`CELLS`: crossing, harbor, headland, greybox, arch_test), the greybox retirements, then the probe file | Integrator |
| `Tools/EditorScripts/pointe_sombre/<cell>.py` | One cell: `build(tk)`, owning only actors tagged `Cell:<cell>` (outliner folder `Cells/<cell>`) | the cell's builder |
| `pointe_sombre/arch_test.py` | The architecture fixture: a door marked TEST by the west end of the quay into an abstracted interior room. Not slice content; it carries `Map.Sombre.Architecture` until the real interior cells and their portals exist | Integrator |
| `pointe_sombre/crossing.py` | The crossing (B0): the *Ida*'s deck, rails, wheelhouse, Liv's letter and Varga's chart, and the one-way wheelhouse door to the quay (sets `sombre.reef_struck` and `sombre.storm`). After the strike the same art lies at `Anchor_IdaBerth` (presence), with static hidden collision built at both places (its rail blocks are `InvisibleWall`, so they never block the interaction trace), and the door is gone | the crossing (Claude, VS-10) |
| `pointe_sombre/harbor.py` | (VS-10) The quay's people: Mara (one actor; rail → quay → store porch by presence) and Varga aboard the moored *Ida* | the harbor (Claude) |
| `pointe_sombre/headland.py` | (VS-10) So far only `Lantern_Crossing`, the false light seen from the deck before the strike (built for distance: it is only ever seen from 345 m) | the headland (Claude; VS-16 grows it) |
| `pointe_sombre/art.py`, `Tools/EditorScripts/dress_sombre_<cell>.py` | (VS-10) Art sublevels: `art.ArtLevel("<Cell>")` links the always-loaded `Lvl_PointeSombre_Art_<Cell>` (created once), clears it, and places NoCollision props (`prop(...)`, stood on the island heightfield); `save()` writes the sublevel, and the persistent map only when the link was just made. A dress script never touches the persistent level's actors, and a core rebuild keeps the link. The first is `dress_sombre_harbor.py` (the quay's piles and clutter), run after the biome in a full rebuild | `art.py` Integrator; each dress script its cell |
| `Tools/PointeSombre/greybox.json`, `pointe_sombre/layout.py`, `pointe_sombre/greybox.py` | (VS-08) The exterior greybox as data, resolved to engine coordinates (pure Python), and its builder: below | Integrator; each cell's builder replaces the pieces tagged `Greybox:<cell>` |
| `Tools/PointeSombre/routes.json`, `pointe_sombre/routes.py`, `pointe_sombre/reach.py` | (VS-08) The route-timing report and the reachable-space flood fill (pure Python): below | Integrator |

**Retiring the greybox (VS-10).** A cell takes over its greybox stand-ins by declaring them, never by editing `greybox.py`: `GREYBOX_RETIRE = ("<label>", ...)` or `GREYBOX_RETIRE_GROUPS = ("<cell>", ...)` (every actor tagged `Greybox:<cell>`). The core removes them after every cell has built; a label that matches nothing stops the build. The headland retires the post's always-lit glow box this way.

A cell script never references another cell's actors. Where two owners meet (a portal and its landing), the core creates a named anchor (`Anchor_<Name>`, tag `Anchor:<Name>`, `anchors_table()` in the core script) before any hook runs, and both sides use `tk.anchor(name)`. VS-04's anchors: `Anchor_NewGame_Deck`, `Anchor_CrossingExit_Quay`, `Anchor_Respawn_TowerBase`. VS-08 added every anchor reserved in the id ledger (`Design/POIs/sombre_ids.md` §9), placed from `greybox.json` by `layout.py`; `Anchor_RemyMarker` is placed but held for VS-20.

### The map frame

- Units: cm in the engine, metres in `island.json`. **X is north, Y is east, Z is up** (Unreal's own axes, so the editor's top view shows north up). Sea level is Z = 0 (the water sheet's top).
- The island: land from X −95 m (the quay) to +105 m, Y −240 m (the west headland's tip) to +196 m (the Pointe). The playable exterior with the reef and the *Ashland Grey*'s reef is about 435 m × 245 m (plan §5.5: 400–500 by 300–400). The crossing deck is offshore at (−330 m, −360 m), facing the island.
- Reference places: quay arrival (−84, 10), store (−40, −5), tower base (22, 160) on the tower rock at 26.5 m, cable hut (80, 95), headland post (−18, −215), *Grey* stern (−118, −228). VS-08 kept this frame: the short axis (245 m, under the plan's 300–400) was not widened, because every route leg times inside its window on it (below).
- The walkable outline (`bounds` in `island.json`) is fenced with hidden walls (`Edge_*`, tag `SombreEdge`): about 5 m into the shallows along the shore, at the quay edge (with a pocket for the *Ida*'s berth), and around the reef causeway and the *Grey*'s reef. VS-08: the walls run from 3 m below the sea to 55 m (above the tower's gallery at 46.5 m; VS-04's 12 m was below the 26.5 m tower pad), on the `InvisibleWall` profile, so they stop the player but not a visibility trace. A hidden floor (`SafetyFloor`, tag `SombreSafetyFloor`) spans the grid at −1.25 m, just under the lowest terrain triangles: off a cliff, where the sea floor has no triangles, the player lands in the shallows inside the fence instead of falling out of the world.
- Interior cell slots (`INTERIOR_SLOTS` in the core): `arch_test` (0, 2500 m, 400 m), `vault` (150 m, 2500 m, 400 m), `net_loft` (300 m, 2500 m, 400 m). That is outside the exterior's XY footprint and far above KillZ.

### The cell placement rule (plan §5.2, decided with evidence in VS-04)

An interior cell:
1. sits in its slot: 2.5 km east of the island and 400 m up. That is beyond every exterior sightline, beyond the 4 km water sheet's edge, out of the lightning's 900 m radius, and high enough that the exterior height fog has thinned to nothing. Interiors stay in the **persistent level**, because presence rules and portals reference them.
2. is enclosed (floor, walls, ceiling).
3. passes every fixed part through `tk.make_interior`. That sets lighting channel 1 only (the exterior sun, fill, moon, and lightning are channel 0, and with shadows off at low spec a directional light would otherwise light a sealed room), and an `LDMaxDrawDistance` of 120 m, so the exterior never draws it. The cell's own lights take channels 0 and 1, so they also light the player's arms and anyone who walks in. Characters stay on channel 0 because presence moves them between the exterior and the cells. Known limit: the exterior sun also reaches the first-person arms and characters inside a cell (no shadows at low spec). This was accepted as it is in the boathouse.
4. has its own bounded post-process volume (priority 10) for its exposure.
5. needs no `AudioVolume`: the exterior ambience is not a 2D bed. It is an `ADCConditionalAudio` with an attenuated, non-spatialized sound (`SA_DC_SombreExterior`: full inside 550 m, silent by 950 m), so it fills the island and the deck and is silent in the slots. A 2D bed would play inside the vault.
6. Ceilings and any downward face use `tk.flat` (a flat-colour material). `M_DC_Surface`'s triplanar top projection gives a downward face an upward normal, so a lamp below cannot light it. The fixture's ceiling rendered black until this was done.

### Terrain: a generated mesh, chosen by a spike

The plan's three candidates were judged on headless regeneration, traces, low-spec cost, and look (VS-04 spike, 2026-10-01):

| Method | Result |
| --- | --- |
| Landscape from a versioned heightmap | **Rejected.** Spawning a `Landscape` from Python in the commandlet that `RebuildContent.bat` uses asserts (`!IsRunningCommandlet()`, the LandscapeEditor module reaches the editor mode manager) and kills the run. It cannot be regenerated headless. |
| Composed library rock and cliff meshes | **Rejected.** The library's `Smugglers_cove` coast rocks and cliffs are Nanite scans of 12–160 MB per mesh, over the project's 12 MB migration ceiling (`ImportPackAssets.ps1`). Nanite does not run under the low-spec DX11 path. A walkable island would need dozens of them, plus hidden collision for paths. |
| **Generated terrain mesh from the same heightfield** | **Chosen.** It regenerates headless from versioned data in about 25 s, or about 3 s when the island is unchanged. Traces and walking meet exactly the rendered surface: 160 probe points, worst error 0.1 cm. It is about 50k triangles over 32 tiles; gate 0 cost is below. |

How the tiles are made (`terrain_mesh.py`):
- the island is cut into 60 m tiles on a 2 m grid
- triangles wholly below −1.2 m are dropped (the water is opaque)
- each triangle takes one of five surfaces, in this order: `_Seabed` (under −0.4 m), `_Shingle` (the splash band, under 1.4 m, even under a trail: VS-08's critic pass found the trail painted along the reef causeway read as a paved bridge), `MI_DC_Sombre_Path` under a trail's walking width (VS-08, `island.json` `paths`), `_Rock` (steeper than 38°), else `_Turf`. These are tinted children of the shore's surface instances. VS-08 fixed a unit error: the height bands compared centimetres with metres, so the shingle band was 1.4 cm deep until then. Once it showed, the gravel shingle mirrored the sky and read as a second sheet of water, so the shingle is now the coast rock, dark. Trails are painted per 2 m triangle, so their edges are saw-toothed (greybox)
- each tile is written as an OBJ with the heightfield's own normals (`Saved/PointeSombreTerrain/`) and imported, with complex-as-simple collision
- a tile whose stored hash (`DCIslandHash` metadata) matches the island data is skipped, so an unchanged island leaves the assets byte-identical
- `FORMULA_VERSION` in `island.py` forces a rebuild when the method changes

Lessons from the spike, kept in the code comments:
- Building a static mesh in Python from a `StaticMeshDescription` gave tiles with no usable normals, whatever the build settings said. `M_DC_Surface` takes its triplanar weights from the vertex normal, so the island rendered black with a sky sheen. Cubes beside it with the same materials rendered correctly, which isolated the cause.
- The OBJ import reads the file as Z-up and only mirrors Y: a point is written (X, −Y, Z).
- Rebuilding a loaded tile in place asserted in a background worker, so a stale tile is deleted and imported fresh.
- The two-sided surface material and the double-sided physics hid a reversed winding: traces reported up-facing normals while the render was wrong. Only a capture shows it.

### Atmosphere: presence is enough (the §6 #8 spike)

Three looks, exactly one active for any combination of flags:

| Look | Active when |
| --- | --- |
| dusk storm | `sombre.hale_arrived` not set and `sombre.meeting_done` not set (a new game) |
| calm evening | `sombre.hale_arrived` set, `sombre.meeting_done` not set |
| night storm | `sombre.meeting_done` set |

Each look is one `ADCConditionalPresence` rule (`Presence_Atmosphere_<look>`) over five actors tagged `Atmosphere:<look>`: its sun (atmosphere sun), a shadowless fill light opposite the sun (the boathouse's answer to no GI at low spec), its sky atmosphere, its height fog, and its post-process volume.
- The active state leaves them as authored.
- The other state hides them **and parks them** at (0, −4 km, −2 km). Hiding is enough for lights, sky, and fog. A post-process volume honours its bounds, not hidden, so the exterior grade is a **bounded** volume (the island, the reef, and the deck), and parking it is what switches it off.
- The rules are not deferred: every change happens while the player is in an interior (the vault at `knows`, the loft at the meeting), and the way back out is a scene cut.
- One shared sky light with real-time capture follows whichever sky is present.
- Lightning is an `ADCFlickerLight` per storm look (tag `Lightning:<look>`), switched by its own world `ActiveConditions`.

**No `ADCConditionalAtmosphere` is needed** (recorded per plan §17). Rain visuals (Niagara or streak cards) and rain or thunder audio are not in VS-04; they are presentation for the crossing (VS-10) and VS-21. `Map.Sombre.Atmosphere` covers every flag combination and a load. The review views `quay_storm`, `quay_calm`, and `quay_night` show the three looks from one camera.

### Player start and respawn

The single `PlayerStart` stands on the *Ida*'s deck (`Anchor_NewGame_Deck`) for a new game. A presence rule (`Presence_Respawn`, not deferred) moves it to the quay when `sombre.reef_struck` is set, and to the tower base when `sombre.vault_opened` is set. `ADCPlayerCharacter::Respawn` already uses the first `PlayerStart`'s current transform, so the existing system is enough and no C++ was added. Presence makes the start movable. `Map.Sombre.Respawn` kills the player in each state and checks where they come back, including after an F9.

### Portal authoring rules (from the rendered VS-04 proof)

- A portal puts the **capsule centre** on its destination marker, so markers stand at **standing capsule-centre height** (floor + 100 cm; the capsule half-height is 96), never floor-snapped. The player then settles about 2 cm onto the floor (`Map.Sombre.Architecture` checks: on the ground, within 3 cm, overlapping nothing).
- A scene cut is global: **every** deferred presence rule in the map commits on it, not only the ones near the portal. Author deferred rules so that committing them at any portal use is acceptable, or keep them out of sight.
- Portals and their markers live in the persistent level (presence and portals cannot reference another level).
- While any portal's transition runs, every portal refuses (VS-04 fix, below), so the way back out may stand right at the arrival point.

### Performance gate 0 (VS-04, 2026-10-01; plan §20)

Taken in one session on this machine, through `Tools\ReviewCapture.bat` with the low-spec `PlayTest` cvars:

| Capture | Views | Average view | Route frames | Peak draw calls | Peak RHI primitives |
| --- | --- | --- | --- | --- | --- |
| `Saved/Review/2026-10-01_0909` (`Lvl_Boathouse`, same session) | 36 | 36.6 ms | 24.3 ms | 492 | 1.74 M |
| **`Saved/Review/2026-10-01_0910_Lvl_PointeSombre` (gate 0, the slice's baseline)** | 8 | **16.7 ms** (every view at the 60 FPS cap) | 21.3 ms | 512–662 | 0.38–0.45 M |

Two consequences for later gates:
- Because every gate-0 view is at the cap, frame time can only show a regression once a view goes over 16.7 ms. Gate 1 compares the cost columns as well.
- The draw-call peaks are high for so few visible components (9 in the quay views). The suspects are the per-frame sky-light capture and the parked atmosphere sets. This is a watch item for gate 1, not a failure.

### Shoreline recipe (Tier C, VS-06; plan §10)

**An agent defines a shoreline zone in data and gets a believable baseline without placing each rock.** The recipe is **scripted HISM**, written headless into an always-loaded sublevel; nothing generates at runtime.

| File | Does | Owner |
| --- | --- | --- |
| `Tools/Biomes/great_lakes_rocky_shore.json` | The recipe: exposure measure, automatic radii, caps, and four families (talus, cobble, driftwood, scrub), each with meshes and their recorded bounds, material, density, clustering, slope / coast-distance / height windows, scale, yaw, tilt, sink, spacing, cull distance, minimum LOD, cap, and `collision: NoCollision`. Every number is a tuning value | the recipe (WP-BIOME) |
| `Tools/Biomes/zones/*.json` | One file per zone: polygons (with `exposure` auto, protected, or exposed), authored exclusions (circle, box, or polygon, in metres, with an optional margin), pads to keep clear by name (`exclude_pads`, joined to `island.json` pads), and (VS-08) trails to keep clear by id (`exclude_paths`: `id`, `margin_m`, joined to `island.json` paths; the trail's half width plus the margin). A file whose name starts with `_` is a fixture: `verify_plan.py` reads it, the map does not (the `Tools/ContentSpecs` rule). The map's zones are `harbor.json`, `cable_hut.json`, `headland.json` (VS-08, written by the Integrator; each becomes its cell owner's file) | each zone's owner |
| `Tools/EditorScripts/biome/plan.py` | Pure Python, the only placement authority: a 2 m lattice, blake2b draws, cluster mask, exposure, windows, exclusions, spacing, caps, seating | the recipe |
| `Tools/EditorScripts/biome/scatter.py` | Loads the map, links `/Game/Maps/Lvl_PointeSombre_Biome` (always loaded; created once), gathers the automatic exclusions from the persistent level, asks `plan.generate`, clears the sublevel, writes one actor per zone (`Biome_<zone>`, tags `Biome`, `Biome:<zone>`) with one HISM per family mesh (tags `Biome:<family>`, `BiomeMesh:<i>`), and writes `Tools/Biomes/out/Lvl_PointeSombre.json` | the recipe; the sublevel and the manifest are build products the Integrator commits |
| `Tools/EditorScripts/biome/verify_plan.py` | Pure checks, no engine: determinism, key order, seed, every rule, every exclusion (and that each authored shape and automatic radius would otherwise have held instances; pads are checked for emptiness only), the exposure of the quay and the headland, the harbor/headland contrast, caps, the NoCollision rule; `--manifest` re-plans the committed manifest from its recorded inputs | the recipe |
| `Tools/EditorScripts/biome/prepare_biome_meshes.py` | One-time preparation of the migrated meshes (after `ImportPackAssets.ps1`), not part of a rebuild: rock Nanite off and default material, the driftwood material's missing defaults | the recipe |

- **Seating.** The planner interpolates the terrain mesh's own triangles (the same diagonal split as `terrain_mesh.py`), so an instance stands on the surface the player walks, not the analytic one. It sits at the lowest of its footprint's corners, sunk by `sink_m`, lifted by the mesh's recorded bounds (`scatter.py` refuses to run if a mesh's real bounds differ from the recipe's).
- **Exposure is open-water fetch,** not coast curvature. Rays leave the point, cross the shallows, and measure open water until land or a reef crest. Pointe Sombre's harbor is sheltered by the offshore south reef, not by a bay: the coast runs clockwise and the vertex nearest the quay is convex, so a coast-turn rule read the quay as exposed (and, with its sign flipped, the headland tip as protected). Mean fetch: about 60 m behind the south reef, 90 m at the quay, over 200 m on the headland, the north shore, and the Pointe.
- **Exclusions.** Authored shapes and pads per zone; automatic radii around every interactable, cell portal, player start, character, location volume, and `BiomeExclude`-tagged actor of the persistent level (whole cm, recorded in the manifest so the plan replays without the engine); hard rejects outside the walkable outline, over a missing terrain triangle, and in the interior slots.
- **Manifest and hash.** Placements are integers (mm, hundredths of a degree, thousandths of scale). `hash` is SHA-1 over the canonical JSON (sorted keys, no spaces) of `planner`, `island_hash`, `inputs` (the recipe and zone fingerprints and the automatic list), `counts`, and `placements`. Integers, so the C++ test re-hashes exactly; SHA-1, because the engine's SHA-256 has no Windows implementation. The `.umap` is not byte-identical between runs (GUIDs); the manifest is.
- **Collision and cost.** Every Tier C component is NoCollision, static, out of navigation, and culled by family. Talus draws from LOD2 and cobble from LOD3: the Megascans rock set keeps scan-density LODs (5-9k triangles per stone at LOD0).
- **Links and rebuilds.** `build_pointe_sombre` never touches the biome sublevel (it clears only persistent actors) and keeps its link; `scatter.py` saves the persistent map only when it had to link the sublevel. A full `RebuildContent.bat` runs `biome\scatter` last. A core-only rebuild can leave the sublevel stale until scatter runs; `Map.Sombre.BiomeExclusions` then fails (the island hash or the instances stop matching).

**PCG or scripted HISM (plan §10.3): scripted HISM.** The spike (2026-10-01, about 5 minutes, fail-fast; `-EnablePlugins=PCG` on the command line only, nothing saved, the project file untouched) failed gate 1. In the `pythonscript` commandlet that `RebuildContent.bat` uses, `PCGComponent.generate()` only schedules the graph (`SCHEDULE GRAPH`); `generated` is still false when the script returns, and the engine logs `ExecutionSource cancelled` at shutdown, because the commandlet never ticks the scheduler. The spawner's mesh list is also read-only from Python, so a graph cannot even be authored headless. Gates 2-4 were not tried. The recipe and zone files are the content; a later PCG tool would read the same files, with `plan.py` as the hash oracle.

**The test zone (VS-06).** Two polygons in `_test.json`, both `exposure: auto`: the sheltered bight west of the quay and the west headland's tip. One authored box on the bight's shore, three pads, and the live automatic exclusions (the deck start, the wheelhouse door, and the arch-test door, which stands inside the bight polygon). 262 instances in 8 components (talus 27, cobble 176, driftwood 18, scrub 41). VS-08 retired it from the map (it is `_test.json`, a fixture now) when the three cell zones took over those shores; it still carries `verify_plan.py`'s contrast and exclusion proofs.

**Cost gate (plan §20: biome on vs off, ≤ 0.5 ms).** This laptop flips between two clock states during a capture (the same view at about 3 ms or about 11 ms), so separate OFF and ON runs, even back to back, differed by 1-9 ms on views the biome cannot touch. The gate is therefore measured inside one run: `-ReviewToggleTag=Biome` hides and shows the biome at each biome view in ABBA windows, each window its median frame and each state its median window (Review capture, above). Two such runs: **+0.18 ms and +0.28 ms** mean over the three biome views. Draw-call peaks rose by at most 16 on the biome views; the quay views are unchanged.

### The exterior greybox (VS-08; Checkpoint A)

The island's shape before anything is dressed. `greybox.py` (cell hook `greybox`, between `crossing` and `arch_test`) reads `Tools/PointeSombre/greybox.json` through `layout.py` and places, every actor tagged `Greybox` (the perf toggle) and `Greybox:<cell>` (the cell that will replace it):
- **Landmarks:** the tower on its pad (base room with a door, a 20 m shaft, gallery with an invisible-wall rail ring, glass lamp room, cap), the west-headland false-light post and its low stone hide, the *Ashland Grey*'s tilted stern on its reef (art NoCollision, hidden deck and hull collision, a ramp from the reef), the Authority mast by the cable hut, and the vault's lower door frame at the tower rock's north foot.
- **Kit shells** (the accepted VS-05 kit through `kit/compose.place`): the store, Odette's cottage, a turned and re-skinned cottage standing in for the Leclair house, the Pruitts' salvage shed, the smokehouse, two harbor sheds, and the cable hut. Each is seated at the lowest ground under its footprint, with its base raised by the footprint's relief, so the kit's pilings meet the rock and no terrain is flattened.
- **Occluders:** three rock outcrops (a library rock scan, NoCollision, with a hidden blocking box inside), placed at least 3 m off every trail, so the tower slides out of view on the climb and is found again.
- **Stub interiors and open stub portals** (labels end "(greybox)"): the vault (20 × 12 × 4 m) and the net loft (the kit's loft shell) in their slots, the tower stair up and down, the hatch, the lower door, the conduit, and the loft stair, each landing on its ledger anchor. Every cell can be walked into at Checkpoint A. The real portals and their conditions are the cells' (VS-12, VS-13, VS-14, VS-18).
- **Location volumes** with their final ids: `sombre.harbor`, `sombre.light`, `sombre.headland`, `sombre.cable_hut`, `sombre.ashland_grey`, `sombre.vault` (none for the settlement or the loft, per the ledger).
- **The *Ida*'s berth:** a fender filling the gap between the quay wall and the moored hull.
Nothing here is narrative: no inspectables, people, or flags.

**Terrain character (VS-08 critic pass).** Three optional `island.json` controls, all deterministic. `folds` is a second, broader noise octave (2.6 m over 30 m cells), so the land rises and dips and hides and reveals; pads stay level. Two more `ramp_zones` (`north shore`, `west head`) make the exposed shores drop in short scarps, so the north and west read as exposed and the harbor bight as sheltered. A `roughness` block on a causeway or reef line varies its crest height and its edge, so the reef causeway is a wandering, broken spine (crest 0.36–0.9 m) that the `head_grey` trail follows. `FORMULA_VERSION` 8.

**Trails.** `island.json` `paths` are the routes (id, centre-line points, half width, falloff, and a maximum grade). The heightfield cuts each trail's cross-slope and caps its grade, the terrain paints it with the path surface, and the route-timing report and the map test walk it.

**Route-timing report** (`py -3 Tools/EditorScripts/pointe_sombre/routes.py [--check]`; writes `Tools/PointeSombre/out/routes.json`). Each §5.5 leg in `routes.json` names a trail. The report samples it every 0.5 m on the terrain mesh's own triangles (`island.MeshSurface`, also used by the biome planner) and gives the walked 3D length and time at the player's 450 cm/s, the climb, the steepest 2 m (the leg fails above 40°), the lowest point, and any point outside the fence or off the mesh. A leg passes within ±20% of its target window. `Map.Sombre.Greybox` then walks the real player along the same trails and times them.

**Reachable space** (`py -3 Tools/EditorScripts/pointe_sombre/reach.py`). A flood fill over the 2 m grid from the quay arrival, on foot (a step between neighbouring cells is allowed under the engine's 44.76° walkable angle). The fence bounds it; ground is the terrain mesh or else the safety floor. It reports reachable void, the places that must be reachable on foot (the store, the tower door, the cable hut, the headland post, the reef by the *Grey*, the vault's lower door, the settlement), and the highest reachable ground against the fence top. Interiors and the gallery are reached by portal; the map test covers them.

**Review views:** `Tools/Review/Lvl_PointeSombre/greybox.json` (`greybox_*`, retired cell by cell as each cell's own view file takes over). The core views and the exterior-cell views trace `tower_site` against `Tower_Shaft`; `greybox_quay_west` traces the false-light post and the *Grey*'s hull from the quay (plan §5.5 landmark overlap).

**Performance gate 1** (VS-08, 2026-10-01, on the final geometry: `Saved/Review/2026-10-01_1826_Lvl_PointeSombre_gate1_final_ab`). This was one uncapped run with `-ReviewToggleTag=Greybox+Biome`, four ABBA cycles of 2 s windows at every view, and lightning hidden. The laptop flipped between its two clock states inside views (windows near 4 ms next to windows near 11 ms), so each view's off and on states are compared within the slow state, the one every view sampled.
- Greybox plus recipe over the 23 exterior views: **median +0.59 ms, mean +0.67 ms**, range −0.38 to +1.75 ms. The two arch-cell controls, where nothing toggled is in view, read +0.18 and +0.77 ms, so the cost is within about 0.8 ms of this machine's noise. An earlier run on the pre-critic terrain (`..._1756_..._gate1_ab`) gave median +0.52 ms.
- The whole map, everything on: 8–13 ms per view in the slow state and 3.6–6.2 ms in the fast one, every view under the 16.7 ms that gate 0 sat at (gate 0 was capped, so it only bounds views from above).
- Route (41 frames, everything on, the trail from the quay to the tower door, on the terrain): mean 10.4 ms, p90 12.7 ms, max 17.1 ms. Gate 0's 21.3 ms route is not comparable (its frames were not proven on the surface; VS-06).
- Visible components per exterior view: 13–167; instances 0–510.
- Draw-call peaks (418–1542) are not a usable cost signal. The same `quay_night` scene read 441 in one run and 1340 in another, so they follow the sky capture and frame timing, not the scene. The VS-04 watch item stands.

### Tests

`DeadCurrent.Map.Sombre.*` (`Source/DeadCurrent/Save/DCSombreArchitectureMapTest.cpp`, Integrator-owned; scratch slot `DeadCurrent_SombreArchTest`):

| Test | Covers |
| --- | --- |
| `Architecture` | a new game on the deck in the storm; the interior lighting rule on the fixture's cell; terrain traces at 160 seeded grid vertices (`terrain_probe.json`): solid, facing up, at the heightfield's height within 2 cm; walking on the terrain up to the test door until the interaction trace focuses it; the prompt; E through the real interactor; the fade sampled each frame (starts clear, only darkens, partly dark, black at the move); movement and look input off; a deferred presence change snapping on the scene cut; arrival at rest, facing the marker's yaw, pitch levelled; the way back refused mid-transition; E and jump mid-transition change nothing; input back after the fade; standing cleanly on the floor; F5 inside the cell, diverge, F9 back into it; a locked portal's verb, refusal, text, and unlocked verb; the way back out |
| `CrossMapLoad` | a shore save loaded from the slice opens the shore with its state; a slice save loaded from the shore opens the slice; a hand-written Phase 5 (version 5) save loads on the shore from the slice |
| `Respawn` | the start on the deck, at the quay after the strike, at the tower base once the vault opens; a death in each state; F9 restores the start |
| `Atmosphere` | exactly one look for each of four flag combinations; each storm's lightning on only in its look; a load restores the night |
| `BiomeExclusions` | (VS-06, `DCSombreBiomeMapTest.cpp`, owned by the recipe) the committed manifest's hash re-computed from its payload; its island hash equals the terrain probe's; the biome sublevel's instances are exactly the manifest's placements (position 2 mm, rotation 0.05°, scale); nothing inside an authored or pad exclusion of its zone (read from the zone files and `island.json`), or within the recipe's automatic radius of a live interactable, portal, player start, character, or location volume; every Tier C component NoCollision, out of navigation, and not on a default material; no instance floats above the terrain. VS-08: also nothing within a zone's `exclude_paths` trails |
| `Greybox` | (VS-08, `DCSombreGreyboxMapTest.cpp`, Integrator) the six location volumes, and no settlement or loft volume; every ledger anchor exists and a capsule stands on collision at it (no exterior spawn in the void); at every fence segment, a pawn pushed outward at 30 m and at 50 m is stopped; every 5 m inside the fence something stops a falling pawn above the sea floor (no void); the real player walks each §5.5 leg's trail to its end, inside ±20% of its window; every stub portal is used (`TryUse` passes) and lands on its anchor, so every interior and the lamp room are reachable from the quay. VS-10: the anchor check ignores pawns (Mara stands on her rail anchor) |
| `Crossing` | (VS-10, `DCSombreCrossingMapTest.cpp`, owned by the crossing and the harbor; there is no `Map.Sombre.Harbor`) zero investment: the door alone strikes, and Mara and Varga are on the quay with no letter and no tie. Then a fresh new game: on the deck in the storm look with no storm flag; the false light burning; the greybox glow retired; Mara at the rail and Varga hidden. The player walks to the hatch cover until the trace focuses the letter, then reads it through the interactor: `liv_letter` once and `watch.mara_travelling`, then the re-read line; the chart. Mara's tie question sets one flag and is asked once, then her watch reveal. F5 on the deck, diverge, F9. The strike through the timed portal: `sombre.reef_struck` and `sombre.storm`, still the storm look, facing up the island, the *Ida* at her berth, the door gone, Mara on the quay, Varga aboard, the false light out. The harbor is discovered once. The player walks to Varga until the trace focuses her over the rail and talks to her with the interactor: her first reply starts both quests, then she waits |

Run rendered (screenshots `Saved/Screenshots/WindowsEditor/SombreArch_*.png`): launch `UnrealEditor.exe` on the map with the `ReviewCapture.bat` cvars and `-ExecCmds="Automation RunTests DeadCurrent.Map.Sombre.Architecture; Quit"`. Screenshots land a frame or more after the request.

## Presentation layer

Presentation Pass (2026-09-29) added art and sound to `Lvl_Boathouse` with no gameplay change. The shape it left behind:

**Art layer.** Non-gameplay dressing lives in the always-loaded sublevel `Lvl_Boathouse_Art`, which `build_boathouse.py` re-links and never edits. `dress_shore.py`, `dress_structures.py`, and `dress_audio.py` own actors in it by tag (`ShoreDress`, `StructureDress`, `ShoreAudio`). Gameplay actors stay in the persistent map, and the script assigns their meshes and materials so a rebuild restores their look. Everything in the art level is NoCollision.

**Materials.** `M_DC_Surface` (triplanar ground, walls), `M_DC_Wreck` (the *Tern* hull), `M_DC_Lake` (open water and the basin slab, default-lit opaque with two slow world-space normal layers; `DeepColor`, `TileCm`), `M_DC_Glow` (unchanged live-water glow), and the Meshy props' own materials. All are generated by `import_art.py`, so a rebuild restores them.

**Props.** The ten Meshy props and the library meshes are `wear_mesh`ed onto the existing inspectables and pickups in `build_boathouse.py` (scaled to the same footprint the interaction trace already hits). Item world meshes are set in `create_items.py`.

**Bodies.** (Phase 5 playtest, 2026-09-30: Mara is now the one-piece `SKM_Quinn_Simple` mannequin in two flat matte colours, `MI_DC_MaraBody` and `MI_DC_MaraTrim`, a PROVISIONAL placeholder; generated heads on the pack body kept reading as a head popping out of the coat. Her head-swap component is inert. The scavenger is unchanged. The rest of this paragraph describes the Presentation Pass setup.) Mara and the scavenger use `SK_Survival_Character` (migrated to `/Game/Survival_Character`, textures at 1K) with the unchanged `ABP_Unarmed`. The pack skeleton lists the mannequin skeleton as compatible, which is what lets that animation blueprint drive it; `import_art.py` sets that on every rebuild. Jacket and jeans slots take `MI_DC_MaraJacket` / `MI_DC_ScavJacket` (and the jeans pair), tints of the pack's own instances. If the pack is missing, `assign_mannequin` falls back to the mannequins.

**Audio.**
- `ADCConditionalAudio` (`World/`) is presentation like `ADCFlickerLight`. It re-checks a `FDCGameplayCondition` list every 0.25 s against the player pawn when there is one (so `HasItem` and `ActorDead` work) and against itself otherwise. `WhileTrue` loops while the list passes and fades out when it stops; `OnceWhenTrue` plays on the rising edge only, so a load that already has the flag is silent. It also snaps on `UDCWorldStateSubsystem::OnRestored` (Phase 5): after a load a loop starts or stops at once with no fade, and a one-shot re-primes without playing. With no conditions and no attenuation it is a 2D bed. It sets no flag and saves nothing.
- Placement: the wind and lap beds are two unconditioned 2D actors in the art level (`dress_audio.py`). The relay hum, live-water hum, and breaker clunk are placed by `build_boathouse.py` because they follow gameplay state: the relay hum plays while the player does not hold `radio_coil`, `shore.relay_recovered` is unset, and `boat.scavenger` is alive; the live-water hum plays while `!wreck.power_cut`; the breaker plays once when `wreck.power_cut` rises.
- `ADCFlickerLight` has optional `FlashSounds`: the six spark lights play one of six snaps on a flash-on with a 30% chance.
- The pistol's `FireSound` and `DryFireSound` are CC0 shots set in `create_items.py`. `DCAudioCues::PlayUI` (`Audio/`) plays a cue by path when a pickup or container item is taken and when the inventory opens; a missing asset is a silent no-op. Cue assets are in `/Game/Interface_And_Item_Sounds`.
- Starting levels come from `C:\FO5_AssetLibrary\Audio\SOURCING_NOTES.md` (wind 0.07, lap 0.46, live hum 0.02, relay 0.06, sparks 0.22, breaker 0.56, pistol 1.0). They are tuned by ear in `import_audio.py` and `build_boathouse.py`.
- `Config/DefaultGame.ini` always cooks `/Game/Audio` and `/Game/Interface_And_Item_Sounds`, because those are loaded by path.

**Bodies, heads, and perimeter (playtest 1).** `UDCHeadSwapComponent` (`Character/`, a default subobject on `ADCFriendlyNPC`, inert until `HeadMesh` is set) hides everything the skeletal mesh skins to the `head` bone and attaches a static head there, so a named character can have her own face on the shared body. Mara uses the Meshy `mara_head`; offset, rotation, and scale are set in `build_boathouse.py` in bone space (X up, Y forward). `M_DC_Current` (`import_art.py`) is the live water: unlit translucent filaments (contour lines of animated noise) over clear water, instead of the flat cyan sheet. The east half of the map is closed by invisible walls (`Edge_*` in `build_west_shore`); `Tools/EditorScripts/inspect_bounds.py` flood-fills the walkable area from the player start and reports any reachable cell bordering void (it reports none).

**A material that fails to compile renders as the engine default and says nothing.** `M_DC_Prop`, the master for every Meshy prop, failed to compile from PP-06 until playtest 1 (its roughness and metallic samplers were the wrong type for the default texture), so every Meshy prop rendered as a pale default material. The pack's eye master also fails; its instance is re-parented onto a plain material. `Tools\ReviewCapture.bat` now fails when the capture log contains `Failed to compile Material`.

**Packaging.** `Tools\Package.bat` runs a Development Win64 `BuildCookRun` to `Saved\Packaged` and a null-RHI smoke launch of each production map. The production maps are the ones registered in `Tools/Maps.bat` (map name and test group; one line per map): `Lvl_Boathouse` and `Lvl_PointeSombre` (VS-04). A registered production map that is not built fails the run, and each smoke log is deleted before its launch, so a stale log cannot pass for a new one. VS-04: both maps cooked (0 errors, 0 warnings) and smoke-loaded. They are cooked explicitly with `-map=` and each is smoke-loaded (`Saved/Logs/PackageSmoke.log` for `Lvl_Boathouse`, `PackageSmoke_<map>.log` for any other); a map missing from the package fails the run. The packaged game target compiles the review capture code, so it must not use editor-only API outside `#if WITH_EDITOR` (`GetActorLabel`).

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
- **Warn** (Phase 5 playtest): when his patrol first notices the player, he stops, faces them, and shows a line (`WarningLine`, PROVISIONAL: "This stretch is mine. Turn around."). He attacks only if they come within `WarnAttackRadius` (6 m) or shoot him. If they leave his sight for `LoseSightTime` he walks his loop again. Once alerted (Investigate) he does not warn again. Standing sight is 12 m (was 18) and his loop is X 2300..3000, Y -60..350, about 16 m from the door.
- **Chase** when sight (18 m, 75° cone) picks up the player, or when shot. A **crouched** player is noticed only within `CrouchedSightRadius` (8 m) and `CrouchedPeripheralDegrees` (45°) of where he faces (Phase 5 playtest; `CanNoticeAt`, re-checked every tick in Patrol and Investigate because the sight sense reports a target only when it first comes into view). Once he is chasing, crouching does not shake him; breaking his line of sight does. At the camp, three crate stacks (`CampCover_*`, hidden collision blocks under closed crates, `build_camp_cover` in `build_boathouse.py`) give the coil route cover outside his patrol square.
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
| `AttributeAtLeast(Id, Quantity)` | raw attribute tag `Id` (e.g. `Attribute.Grasp`) is at least Quantity |
| `SkillAtLeast(Id, Quantity)` | **effective** skill tag `Id` (e.g. `Skill.Engineering`) is at least Quantity |
| `HasPerk(Id)` | perk tag `Id` (e.g. `Perk.SchematicEye`) is owned |

Every condition has `bNegate`. A list passes when every entry passes; an empty list passes.

| Consequence | Does |
| --- | --- |
| `GiveItem` / `RemoveItem(Id or Item, Quantity)` | instigator's inventory |
| `StartQuest(Id, Stage?)` | start at Stage, or at the quest's start stage |
| `SetQuestStage(Id, Stage)` | move stage (to an outcome stage = complete) |
| `SetWorldFlag` / `ClearWorldFlag(Id)` | world state |

`FDCRuleContext` carries what rules read and write: instigator, its inventory, quest log and `UDCCharacterProgressionComponent`, the world's `UDCWorldStateSubsystem` and `UDCPersistentRegistry`. Missing pieces fail conditions and skip consequences safely. `UDCGameplayRules::CheckConditionsFor` / `ApplyConsequencesFor` are the Blueprint entry points. `ValidateReferences` reports unknown quests, stages, items, attributes, skills and perks (used by `DeadCurrent.Content.Validate`). `FormatCheckLabels` turns the build checks in a list into text such as `[Engineering 2] [Schematic Eye]`. Dialogue prepends that to a visible choice. An inspect prompt prepends it when the active variant required a build check.

**Failed build checks are hidden.** A dialogue choice whose conditions fail is not shown. An inspect variant whose conditions fail is skipped and a later variant, or the default text, is used. There is no greyed-out row in this phase.

## Character progression

`UDCCharacterProgressionComponent` (`Character/`) on the player owns the build. `ADCPlayerCharacter` does not grow a field per skill. Lookup is by Gameplay Tag. Unknown tags read as 0 or not owned and cannot be spent.

PROVISIONAL model, chosen because each attribute is the capacity behind one skill and a later skill is another row:

| Attribute | Linked skill | Attribute is |
| --- | --- | --- |
| `Attribute.Grasp` | `Skill.Engineering` | reading made things |
| `Attribute.Fieldcraft` | `Skill.Survival` | reading ground, animals, and how people moved |
| `Attribute.Bearing` | `Skill.Persuasion` | pressing for what someone withheld |

Effective skill = invested ranks + 1 when the linked attribute is at least 2, else + 0. `SkillAtLeast` uses the effective value. `AttributeAtLeast` uses the raw attribute. Perks are owned or not.

Pools on a new game, and on any save from before version 5: 3 attribute points (max 2 each), 2 skill points (max 2 each), 1 perk. Unspent points are the pool minus what is invested; they are not stored separately. **F10** on the build panel refunds everything. That reset is a prototype so a playtest can try each orientation. It is not a respec feature.

Perks, each used by an inspect variant: `Perk.SchematicEye` (Sounder Chart against the depth sounder), `Perk.PulseRead` (dead fish), `Perk.RelayEar` (relay housing, after it has been inspected).

The catalog is the static table in `DCCharacterProgressionComponent.cpp`, next to the native tags. To add one later: add the tag, add a row (and the attribute link, for a skill), and author `SkillAtLeast` / `AttributeAtLeast` / `HasPerk` on existing content. Do not branch actors on a skill by name.

Tags for this set live in `DCCharacterProgressionComponent.cpp` (`DCProgressionTags`), not `DCGameplayTags.h`, because nothing else should mention them except through the catalog and the rule conditions.

**Adding a condition or consequence type** (skill check, reputation, companion present, discovered info): add the enum value and any field to `DCGameplayTypes.h` (use `EditCondition` so the editor shows only relevant fields), a case in `CheckCondition` / `ApplyConsequence` and in `ValidateReferences`, any new data the rule needs to `FDCRuleContext::ForActor`, and a test in `DCGameplayRulesTest.cpp`. Dialogue, quests and inspectables pick it up with no changes.

## World state

`UDCWorldStateSubsystem` (`World/`, world subsystem) owns named world flags (`shore.path_cleared`) and the `OnChanged` signal. Flags are set by consequences and read by conditions. `NotifyChanged` fires on flag changes and on any health-component death; quests re-check their transitions on it. Flags are saved and restored with the game. Name flags `<area>.<fact>`. The subsystem also holds `DiscoveredLocations`: `DiscoverLocation(Id)` returns true only the first time and fires `OnLocationDiscovered` and `OnChanged`; `IsLocationDiscovered(Id)`; `ReplaceDiscoveredLocations` (silent, used by load).

`OnRestored` / `NotifyRestored()` (Phase 5) says "state was replaced wholesale": `UDCSaveSubsystem::ApplyPendingLoad` fires it after the world, the player, and the quest log are applied, and the review capture fires it after a viewpoint's setup. It is not a change signal and nothing re-runs consequences on it; presentation that normally changes out of sight (conditional presence) snaps on it.

## Conditional presence

`ADCConditionalPresence` (`World/`, Phase 5) is the rule "these actors are here, somewhere else, or not here at all, when these conditions pass". It is a rule actor placed where its targets are authored; it is their pivot.

- `Targets`: actors in the same level. Each keeps its offset from the rule actor's authored transform (captured at BeginPlay).
- `States`: ordered `{StateId, Conditions, bPresent, bMove, Placement}`. The first state whose `FDCGameplayCondition` list passes wins. None passing: authored transforms, present. An empty condition list always passes, so an unconditioned last state is an explicit default ("hidden unless ...").
- `bMove`: the pivot takes `Placement` (world transform) and each target follows with its offset. Targets that move must be **Movable** (a warning is logged otherwise).
- `bPresent = false`: targets are hidden in game with collision off, so traces, bullets, and the player pass through. Nothing is destroyed; registry and save are untouched.
- Conditions run against the local player's pawn when there is one (item, quest, and build checks work), otherwise against the rule actor, as in `ADCConditionalAudio`.
- Evaluated at BeginPlay (snap), on `OnRestored` (snap), on `OnChanged` (flags, deaths, discoveries), and every `CheckInterval` (0.5 s; quest and inventory changes).
- **Deferral** (`bDeferWhileObserved`, on by default): a change that is not a snap waits while the player is within `ObservedDistance` (15 m) of the pivot's current or next place, or while a target was on screen in the last half second (`GetLastRenderTime`; a world that never rendered does not count). So a character does not vanish mid-conversation and nothing appears at the player's feet. `HasPendingChange()` reports it; `Snap()` forces it; `SetObserverOverride` lets tests and tools judge "observed" from a point.
- It **saves nothing and sets nothing**. Its result is recomputed from state that is already saved, so a save from before a rule existed shows whatever its flags imply. Do not use it for something that must be remembered on its own (an object the player moved, a door left open); that is `IDCPersistent`.
- `GetActiveStateId()` (`NAME_None` for the default) is what tests and review read.

Test: `DeadCurrent.World.ConditionalPresence` (default, first match, move with offsets and yaw, hide disables collision and re-show restores it, deferral near the current and the new place, restore snaps, replaced flags followed on restore, a destroyed target skipped, no flag written, no persistent id).

Authoring from a map script: spawn the rule at the targets' authored pivot, set `targets` to the actor list and `states` to a list of `DCPresenceState`. One rule per group that changes together (the skiff and its cargo move as one).

Presence also snaps on a **scene cut** (`OnSceneCut`, below): a cell portal moves the player while the screen is dark, so a change waiting for the player to look away applies then.

## Cell portal (Phase 6)

`ADCCellPortal` (`World/`, VS-03) moves the player between two places in the **same map**: an exterior and an interior cell built elsewhere in the level (a vault under a lighthouse, a loft over a store), or the bottom and top of a stair. Phase 6 keeps interiors in one map instead of using map travel because flags, quests, the registry, and the build are per-world and a save holds one map's actors (`VerticalSlicePhasePlan.txt` §5).

| Property | Meaning |
| --- | --- |
| `Mesh` | what the interaction trace hits: the door, hatch, or stair |
| `DisplayName` | the prompt's target (`[E] Unlock Vault hatch`) |
| `Variants` | ordered `FDCPortalVariant { VariantId, Conditions, Verb, Consequences }`; the first whose conditions pass for the interactor is used, exactly like inspect variants. An unconditioned last variant is the open state. A variant whose conditions include a build check gets the usual `[Engineering 2]` label in its prompt. |
| `LockedVerb`, `LockedText` | prompt verb (default "Try") and the message shown when no variant passes. Nothing else happens. This is how the slice does locked access: there is no separate locked-door class. |
| `Destination` | an actor in the same level (usually an empty marker); the player arrives at its location, facing its yaw when `bUseDestinationYaw` (pitch levelled) |
| `FadeOutSeconds`, `HoldSeconds`, `FadeInSeconds` | the transition (defaults 0.35 / 0.15 / 0.35). All three zero: **instant**, completing synchronously inside the use (tests and tools) |
| `CardText`, `CardSeconds` | optional line shown while the screen is dark ("You climb the stair."), as a timed HUD message |

A use (`Interact`, or `TryUse`, which returns `EDCPortalUse::Locked / Ignored / Passed` for tests and tools):

1. A transition already running on **any** portal in the world (VS-04: the way back out often stands at the arrival point; a second portal must not start mid-fade), or no `Destination` (an error is logged): ignored. `CanInteract` is false meanwhile, so no prompt shows.
2. No variant passes: `LockedText`, nothing else.
3. Otherwise the screen fades out with movement and look input off. At black the portal:
   - ends any conversation
   - applies the chosen variant's consequences (through the shared rules, for the interacting player)
   - moves the player to the destination at rest (velocity zeroed, control rotation set)
   - broadcasts `UDCWorldStateSubsystem::NotifySceneCut()`
   - shows `CardText`

   Then it holds, fades back in, and gives input back. `CanInteract` is true whenever no portal's transition is running, including while locked, so the player can read why.

**The scene cut** (`OnSceneCut` / `NotifySceneCut()` on the world-state subsystem) means "the player was moved between scenes while the screen was dark". Conditional presence snaps on it (so the net loft can fill as the player climbs the stair, even though they arrive inside the attendees' 15 m bubble). It is **not** a restore: `OnRestored` means "state was replaced by a load; do not replay anything". Anything that must play once in ordinary play but never on a load (a one-shot sound, a light's one-time sequence) treats a scene cut as ordinary play and a restore as a snap.

Authoring from a map script:
- spawn the portal at the doorway with its mesh
- set `variants` (list of `DCPortalVariant`), `destination`, `display_name`, and `locked_text`
- one portal per direction: a hatch and its way back are two portals
- when a portal and its destination belong to different owners (the hatch in the tower base, its landing inside the vault), both sides use the shared anchor actors the core map script creates (`Design/POIs/README.md`)

**It saves nothing.** It has no persistent id and is not `IDCPersistent`. Its lasting effects are the flags its consequences set and the player's position, both already saved. Do not use it for anything that must be remembered on its own. Known edge: a save taken during the fade records the player's position at that instant (before or after the move) and whatever consequences have already run. Both are consistent states, and a load always ends any transition, because the map reopens.

Test: `DeadCurrent.World.CellPortal` (editor context, no map). It covers:
- locked text and prompt, and first-match variants and their prompts
- consequences once per use, through the real rules
- arrival at the destination's location and yaw, at rest
- the scene cut snapping a deferred presence change (a plain flag change still defers)
- `NotifySceneCut` and `NotifyRestored` never standing in for each other
- the timed transition: nothing happens before black, uses are ignored while it runs, it is usable again after the fade in
- no destination is refused
- while one portal's transition runs, another portal offers no use and is ignored, and works again afterwards (VS-04)
- it has no persistent id and is not `IDCPersistent`

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

To add a quest: add a spec file to `Tools/ContentSpecs/quests/` (stages, transitions, OnEnter) and its dialogue to `Tools/ContentSpecs/dialogue/` (see "Content specs"), place any actors or inspectable variants in the map script, run `Tools\RebuildContent.bat`, then `Tools\RunTests.bat` (Content.Validate catches broken references).

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

Boathouse IDs: `boat.scavenger`, `boat.mara`, `boat.door`, `boat.pickup_pistol`, `boat.pickup_ammo`, `boat.pickup_dressing`, `boat.pickup_coil`, `boat.wreck_locker`, `boat.wreck_tender`, and (Phase 5) `landing.tackle`. Reserved and unused: `landing.crate`, `landing.skiff`, `landing.note`.

`UDCSaveGame` is the slot (`DeadCurrent`, user 0; tests switch to a scratch slot with `UDCSaveSubsystem::SetSlotName`). `UDCSaveSubsystem` (`UGameInstanceSubsystem`) writes player transform, health, inventory, equipped magazine, the quest log (quest id + stage), world flags (from `UDCWorldStateSubsystem`), and every registered persistent actor. World-actor inventories are stored as parallel primitive arrays (`WorldInvActorIds` / `WorldInvItemIds` / quantities / paths) because nested `TArray` stacks inside `WorldActors` or `ActorInventories` can serialize empty through `USaveGame`.

F5 saves. Saving while dead is refused ("You can't save now."). F9 reads the slot, **reopens the saved map** (`MapPackage`, else `MapName`), and `ADCGameMode::StartPlay` calls `ApplyPendingLoad` once every actor has begun play. Loading therefore always starts from a fresh map (taken pickups come back, dead enemies are alive) and then applies the save, so a load inside a running session behaves exactly like a load after relaunching, loading while dead or mid-dialogue is clean, and loading an earlier save can never lose an item the world already destroyed. Apply order: world flags and discovered locations first (silently, so discovery volumes and world-conditioned hazards already see them and nothing is re-announced), then world actors (apply saved actor state, then destroy only ids in `RemovedPersistentIds`; restore deaths, which marks the health component dead so corpses stay lootable and `ActorDead` holds; then corpse and container inventory from the flat arrays), then the player (transform, health, inventory without a magazine refill, character build), then the quest log. Restoring never re-runs stage `OnEnter` consequences.

A persistent actor that is simply absent from `WorldActors` keeps its authored state. That is how a pickup or container added to the map after a save was written still appears. Removal is explicit: when a persistent actor is destroyed during play (`EndPlay` reason `Destroyed`), `UDCPersistentRegistry` records its id, and the save writes that list to `RemovedPersistentIds`. Map travel does not record removals.

Save format version (`UDCSaveGame::SaveVersion`, current 5): 0/1 = first playable; 2 adds `MapPackage`, world flags owned by the world-state subsystem, and data-driven quest stages; 3 adds `DiscoveredLocations`; 4 adds `RemovedPersistentIds`; 5 adds `Attributes`, `Skills` and `Perks` (tag name + rank, or a perk tag name). Older saves load with no discoveries when they predate version 3. Saves before version 4 have an empty removed list. For those, load still treats a *known* boathouse pickup as taken when it is absent from `WorldActors`: `boat.pickup_pistol`, `boat.pickup_ammo` and `boat.pickup_dressing` for every older save, plus `boat.pickup_coil` from version 2. Any other absent id (a pickup added to the map later) is left alone. Saves before version 5, and a version 5 save that spent nothing, both restore the unspent build. Load replaces the build from those arrays; it does not merge. A saved quest stage that no longer exists in the quest data is dropped with a warning so the quest can be taken again. `MapName`/`MapPackage` route the load to the right map; there is still only one production map.

Automation tests `DeadCurrent.Save.PersistentId`, `DeadCurrent.Save.InventoryRestore`, `DeadCurrent.Save.WorldInventorySlot`, and `DeadCurrent.Save.RemovedPickup` cover lookup, inventory snapshot restore, USaveGame round-trip of corpse loot, and the taken-versus-added-later pickup cases. `DeadCurrent.Quest.Persistence` covers the quest log and flags, and `DeadCurrent.Map.Boathouse.*` covers full F9 loads in the real map.

`ADCHUD` is a temporary canvas HUD: crosshair dot, interaction prompt, timed messages via `ADCHUD::ShowMessageFor`, the inventory panel with the quest journal beside it (quests, places, and a character block), the build panel (**B**), the weapon ammo readout, a health bar, the dialogue panel, and the tracked quest objective. The interaction prompt, inspect messages, discovery banner, and objective sit on a dark plate so light text stays readable on a pale wall. Inspect messages wrap to the same width as dialogue. It will be replaced by UMG widgets when the HUD grows. Dialogue choices that passed a build check show that check in front of the line.

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
| `Character/` | Player character, progression component, player controller, camera manager |
| `Interaction/` | Interaction interface, detection, prompts |
| `Items/` | Item definitions |
| `Inventory/` | Inventory runtime |
| `Combat/` | Weapons, damage processing, health |
| `AI/` | Enemy controllers, perception, behavior |
| `Dialogue/` | Dialogue data and runtime |
| `Quest/` | Quest definitions and the player quest log (conditions/consequences live in `Core/`) |
| `Save/` | Save game, persistent IDs, persistence interfaces |
| `UI/` | HUD and widget base classes |
| `Audio/` | Presentation cue helper (`DCAudioCues`): a sound played beside an existing action, by path, silent when the asset is missing |
| `World/` | Persistent world objects, inspectables, `UDCWorldStateSubsystem` (world flags), conditional presence (`ADCConditionalPresence`), the cell portal (`ADCCellPortal`), and the cosmetic presentation actors (`ADCFlickerLight`, `ADCConditionalAudio`) |

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
| `Attribute.` | Character attributes (`Attribute.Grasp`, `Attribute.Fieldcraft`, `Attribute.Bearing`) |
| `Skill.` | Character skills (`Skill.Engineering`, `Skill.Survival`, `Skill.Persuasion`) |
| `Perk.` | Character perks (`Perk.SchematicEye`, `Perk.PulseRead`, `Perk.RelayEar`) |

Later roots from the long-term plan: `Faction.`, `Status.`, `Quest.`, `WorldPower.`. `Skill.` is in use.

Rules: PascalCase segments, singular nouns, no tag without a consumer.

## Template origin

The project was scaffolded from the UE 5.8 First Person C++ template (Horror and Shooter variants excluded). The template classes were renamed and `Config/DefaultEngine.ini` `[CoreRedirects]` maps the old `/Script/TP_FirstPerson` classes to the new ones so template Blueprints keep working. Once those Blueprints have been re-saved in the editor, the redirects can be removed.

| Template class | Project class |
| --- | --- |
| `ATP_FirstPersonCharacter` | `ADCPlayerCharacter` |
| `ATP_FirstPersonGameMode` | `ADCGameMode` |
| `ATP_FirstPersonPlayerController` | `ADCPlayerController` |
| `ATP_FirstPersonCameraManager` | `ADCPlayerCameraManager` |
