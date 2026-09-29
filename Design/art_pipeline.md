# DEAD CURRENT - Art and Audio Pipeline

Living document. Read it before importing, generating, or placing any art or audio. Update it when the first real asset lands and a convention below stops being a proposal.

Status (2026-09-29): the art-layer decision is in force (`Lvl_Boathouse_Art`). `import_art.py` has brought in the CC0 shore surfaces and `M_DC_Surface`. The persistent map is still greybox until those instances are assigned. No audio yet. Conventions still marked **Proposed** have not been exercised.

## Art direction (from `LongTermPlan.txt` §18)

Grounded stylization, not maximum photorealism. Strong silhouettes and a consistent palette matter more than asset detail.

- cold freshwater, black rock, white birch, dark pine
- oxidized ship hulls, faded maritime paint, decaying industrial structures
- industrial lighting; violent blue-white electrical storms (the Current)
- mood: isolation + curiosity + danger + mystery

Everything below is judged against this. A beautiful asset that reads as tropical, European, or branded is the wrong asset.

## The asset library: `C:\FO5_AssetLibrary`

About 39 GB, assembled for an earlier project ("FO5", Cleveland-themed). **Anything in it may be used for DEAD CURRENT.** It lives outside this repo. Copy in only what is used; don't point the project at it.

### 1. UE 5.8 marketplace project: `C:\FO5_AssetLibrary\FO5_AssetLibrary\`

`FO5_AssetLibrary.uproject`, same engine version as ours. Bring assets across with the editor's **Asset Actions > Migrate** from that project into `C:\deadcurrent\Content`. Migrate copies dependencies and keeps the pack's folder name (`/Game/AbandonedPowerPlant/...`), which matches our rule that third-party content stays in its original folder. **Migrate individual assets, never a whole pack.** Several packs are multi-GB and every `.uasset` goes through Git LFS.

| Pack | Size | Fit | Use for |
| --- | --- | --- | --- |
| `AbandonedPowerPlant` | 0.9 GB | Strong | Industrial and hydro structures, pipes, panels, machinery. Relay and power-station dressing. |
| `Scene_Junkyard` | 3.4 GB | Strong | Rusted vehicles, scrap, debris. Scavenger camp. |
| `Scene_UnfinishedBuilding` | 3.1 GB | Good | Concrete shells, scaffolding, decaying interiors. |
| `DriftWoodPack` | 1.5 GB | Strong | Shoreline dressing. |
| `SM_Notes` | 0.2 GB | Strong | Paper notes, pages. Clue props (survey log, chalk-adjacent notes). |
| `Survival_Character` | 1.4 GB | Strong | Modular survivor (jacket, jeans, gloves, backpack) on the **UE mannequin skeleton** (`IK_Mannequin`, `RTG_Mannequin`), so our existing mannequin animations retarget. First candidate for Mara and the scavenger. |
| `Interface_And_Item_Sounds` | 36 MB | Strong | UI, pickup, and inventory sounds. |
| `ModularBuildingSet` | 0.7 GB | Check | Contents not yet surveyed. |
| `ForestLandscape` | 0.9 GB | Check | Landscape materials and meshes. Check species against birch and pine. |
| `EuropeanBeech` | 7.0 GB | Weak | Deciduous hardwood. Usable as background forest only. Not birch or pine. |
| `Smugglers_cove` | 13 GB | Poor | Mostly **tropical** foliage (anthurium, calathea, pachira). Rocks, cliffs, and some props may be usable. Avoid the plants. |
| `Megaplant_Library` (ginkgo) | 0.1 GB | Poor | Wrong region. |
| `MSPresets` | 12 MB | Utility | Megascans master materials; migrate as a dependency, not on purpose. |

### 2. Raw Fab / Megascans scans: `C:\FO5_AssetLibrary\Fab\`

FBX + **8K** texture sets (AO, BaseColor, Normal, Roughness, …) with Megascans JSON metadata. Fab Standard License: fine in a shipped UE game.

- `rubble_pack`, `concrete_rubble_pack`, `wood_debris_pack`, `tree_debris_pack`, `bundled_rope_coil`
- `harbor_props/`: anchor, bollard, chain, chain hook, mooring cleat, rope, ship's searchlight, single post. Low-poly FBX, **no textures**. Material them from the CC0 set (`rusty_painted_metal`, `metal_grate_rusty`).
- `motorboat_wreck/`: a white motorboat wreck, 1.7 MB FBX + two ~30 MB diffuse maps. Strong candidate for the *Tern* or another wreck.

Import at **2K max texture size**, never 8K (see Budgets).

### 3. CC0 textures and models: `C:\FO5_AssetLibrary\CC0\`

Poly Haven (23) and ambientCG (3), all CC0. Each has a `source.json`. Normal maps are DirectX (`nor_dx` / `NormalDX`), which is what Unreal expects.

- Surfaces: `coast_rocks_01`, `coast_land_rocks_02`, `coast_sand_02`, `brown_mud_02`, `frozen_lake`, `chipped_concrete`, `broken_brick_wall`, `rustic_stone_wall`, `blue_plaster_weathered`, `rusty_painted_metal`, `metal_grate_rusty`, `clay_roof_tiles`, `CorrugatedSteel009`, `Gravel008`, `Rope001`
- Models (glTF 2K): `life_jacket` (the Survey Launch has cut life jackets), `Lantern_01`, `hanging_industrial_lamp`, `can_rusted`, `wooden_crate_01`, `binder_notebook`, `dead_tree_trunk`, `dead_quiver_branch_01`, `tree_stump_01/02`
- HDRI: `beach_cloudy_bridge_4k.hdr`, for lookdev and a sky reference.

### 4. Audio: `C:\FO5_AssetLibrary\Audio\Gunshots\`

8 WAVs from Freesound, all **CC0**, each with a `source.json` (author, URL, loudness, intended use): revolver `.38`, heavy pistol, carbine, Enfield .303, field sniper, a distant NPC shot, and two dry-fires (revolver, striker). No ambient, water, wind, or electrical audio yet. That gap matters most for the Current and the live water.

### 5. Earlier Meshy outputs: `C:\FO5_AssetLibrary\Meshy\`

Each folder has `model.fbx`, `model.glb`, 4 PBR PNGs (base color, metallic, roughness, normal), a thumbnail, and a `source.json` recording prompts, task ids, polycount, and credits.

- Usable: `lighthouse_logbook` (survey log, lighthouse slice), `canned_rations_tin` (loot), `salvaged_jacket`.
- Off-theme: `bronze_gridiron_trophy`, `electric_guitar_relic` (Cleveland-specific).

### 6. Reference photos: `C:\FO5_AssetLibrary\Reference\`

Cleveland pierhead lighthouses (east and west), Municipal Stadium, Huntington Bank Field, Rock Hall, West Side Market. Licenses are **CC BY / CC BY-SA**. **Reference only**: never ship them or use them as textures or as Meshy image inputs. The pierhead lighthouses are useful reference for the vertical slice's lighthouse. Don't reproduce identifiable or branded real buildings.

## Meshy Pro (API)

Use Meshy for **bespoke story props** that no pack has: things the player inspects and that carry a clue. Don't use it for generic dressing the library already covers (rocks, debris, crates, foliage), or for anything large, modular, or walkable. Current candidates are the greybox clue actors: the Maritime Authority relay housing, depth sounder, Sounder Chart, breaker panel, battery bank, emergency beacon, and the *Tern*'s flaked name board.

### Access

- The account is Anthony's Meshy **Pro** subscription.
- The API key is in `C:\FO5_AssetLibrary\Meshy\meshy_key.txt`. Read it at runtime (`Authorization: Bearer <key>`). **Never print it, log it, put it in a command line that gets echoed, commit it, or copy it into this repo.** A generation script in `Tools/` reads it from that path or from a `MESHY_API_KEY` environment variable.
- Every task spends paid credits. The five earlier props cost 30–50 credits each (preview + refine). Batch-generating is a decision for Anthony: say what you intend to generate and roughly what it costs before running more than a couple of props.

### Workflow (text to 3D, as used for the earlier props)

API reference: https://docs.meshy.ai. Check it before scripting; model names change.

1. `POST https://api.meshy.ai/openapi/v2/text-to-3d` with `mode: "preview"`, `prompt` (≤ 800 chars), `ai_model: "latest"`, `target_polycount`, optionally `topology`, `should_remesh`. Geometry only.
2. Poll `GET /openapi/v2/text-to-3d/{id}` until it succeeds. Look at the thumbnail. If it's wrong, rewrite the prompt instead of refining a bad preview.
3. `POST` again with `mode: "refine"`, `preview_task_id`, `texture_prompt`, `enable_pbr: true`.
4. Download FBX (or GLB) and the PBR textures from `model_urls` / texture URLs.

Lessons already recorded in the earlier `source.json` files:

- State real-world size in the prompt ("about 25 by 18 by 4 centimeters"). It anchors proportions.
- Thin shells (open book pages, cloth edges) can fail with `OverDenseInputError`. Simplify the shape (closed book, garment lying flat).
- For garments, say "no body inside, no mannequin".
- Hand-held props: 6k–10k target polycount. Larger inspectables: up to ~20k. Nothing is a hero close-up at 300k.

Prompting for DEAD CURRENT: the earlier prompts said "post-apocalyptic wasteland ... photorealistic". Our look is **grounded, stylized, cold, maritime, and industrial**. Name the materials and palette instead: oxidized steel, faded maritime paint, freshwater stains, Great Lakes, 1970s–2000s municipal and maritime hardware. Leave out "wasteland", Fallout-isms, and brands. The Maritime Authority's look is PROVISIONAL, so ask Anthony before fixing a logo or livery on a prop.

### Record keeping

For every generated asset, keep the raw output outside the repo, in `C:\FO5_AssetLibrary\Meshy\<snake_case_id>\` (same layout as the existing folders). That includes `source.json` with: `id`, `date`, `ai_model`, `target_polycount`, every prompt tried (mark failures and why), `texture_prompt`, `task_ids`, `credits_used`, and `generated_with: "generated with the owner's Meshy Pro account"`. Only the imported, downsized result enters `Content/`.

## Bringing assets into the project

### Budgets

`Tools\PlayTest.bat` targets a low-spec machine: DX11, Low scalability, **400 MB texture pool**, no Lumen. Art has to look right there.

- Textures: **2K max** for props and surfaces, 1K for small props. Set `MaxTextureSize` or LOD group on import. Never import the 8K Megascans maps at full size.
- Use packed ORM (occlusion/roughness/metal) where the source allows it.
- Nanite is fine for static environment meshes. Keep simple collision on everything the player or bullets touch.

### Gameplay constraints art must respect

- **Generated maps.** `build_boathouse.py` regenerates the persistent `Lvl_Boathouse` and wipes hand edits there. **Decision (2026-09-29):** dressing that is not a gameplay actor lives in the hand-authored streaming sublevel `Lvl_Boathouse_Art` (`/Game/Maps/Lvl_Boathouse_Art`, always loaded). The script re-links that level on every rebuild, destroys only persistent-level actors, and does not save the art level after the one-time create. A sentinel actor tagged `ArtLayerSentinel` is the proof: `Tools\RebuildContent.bat` must leave it in place, and `DeadCurrent.Map.Boathouse.ArtLayer` must find it. Gameplay actors (inspectables, containers, the door, the water slab, the damage volume, sparks, NPCs, pickups) stay in the persistent map, and the script assigns their meshes. Mood (sun, sky, fog, post process) stays in the script too, so the art level does not add a second sun. Art-level meshes are NoCollision unless a task writes down why a specific mesh needs simple collision.
- **Collision is gameplay.** Firearm hitscan traces WorldStatic/WorldDynamic, AI navigation is rebuilt at runtime, and location discovery is polled (not collision-based) for exactly this reason. Dressing meshes change cover, sightlines, and nav. Rerun `Tools\RunTests.bat` (the `Map` suite walks real routes) after dressing a playable space.
- **Clue actors are inspectables** (`ADCInspectableActor`) with persistent ids and world-state variants. Replace their greybox mesh; keep the actor, id, and variants. The glow and live-water visuals switch from world flags (`M_DC_Glow`, `ADCDamageVolume.ActiveConditions`). A new material must keep that switch working.

### Import

- UE-pack assets: Migrate from the library project (above). Not started.
- Raw FBX/glTF/texture sets: `Tools/EditorScripts/import_art.py`, re-runnable, registered first in `Tools\RebuildContent.bat`. A second run replaces textures in place (27 surface textures before and after).
- Destination: `/Game/Art/<Source>/<AssetId>/` (`PolyHaven`, `AmbientCG`, and later `Megascans`, `Meshy`, `Freesound`). Names use `T_` / `MI_` / `SM_`.
- `M_DC_Surface` is a default-lit master. Base color and roughness are triplanar in world space so a scaled greybox cube does not stretch them. `TileSizeCm` is the repeat, in centimeters. `PackedORM` uses an ARM texture (R occlusion, G roughness, B metal), which matches the Poly Haven layout. Unpacked sets use a roughness texture and the `Metallic` scalar. The normal map is sampled in world XY and faded out on vertical faces, so floors keep detail and walls keep the mesh normal.
- The boathouse script resolves `MI_DC_CoastRock`, `MI_DC_LandRock`, `MI_DC_CoastSand`, `MI_DC_Mud`, `MI_DC_Concrete`, `MI_DC_Plaster`, and `MI_DC_Steel` before it saves. It does not assign them yet.
- Fab meshes (the *Tern*) are imported by the same script when they are placed. They are not in the repo until then.
- Baseline screenshots are not in this step. `UnrealEditor-Cmd` crashes in `take_high_res_screenshot` (null RHI, and again with `-AllowCommandletRendering`). The six cameras are listed in `Tools/EditorScripts/capture_presentation.py`. They get captured from the game window at the start of the mood task, before the lights change.

## Review before playtest

Every visual change is captured and reviewed before a session report is marked **READY FOR ANTHONY TO TEST**.

Run `Tools\ReviewCapture.bat` (close the editor first). It writes PNGs, `contact_sheet.png`, and a UTF-8 `manifest.json` to `Saved/Review/<yyyy-mm-dd_hhmm>/`, from the viewpoints and expectations in `Tools/Review/Lvl_Boathouse.json`, at the same settings as `Tools\PlayTest.bat`. A review reads those captures against the written expectations and the automatic checks. That happens before Anthony is asked to play. His time is for how the shore feels.

### Provenance

CC0 needs no credit. The trail stays here. Fab and Meshy assets, when they arrive, are licensed to Anthony's accounts.

| Asset | Source path | License | Author |
| --- | --- | --- | --- |
| `T_coast_rocks_01_*`, `MI_DC_CoastRock` | `CC0/polyhaven/coast_rocks_01` | CC0 | Rob Tuytel, Rico Cilliers |
| `T_coast_land_rocks_02_*`, `MI_DC_LandRock` | `CC0/polyhaven/coast_land_rocks_02` | CC0 | Rob Tuytel, Rico Cilliers |
| `T_coast_sand_02_*`, `MI_DC_CoastSand` | `CC0/polyhaven/coast_sand_02` | CC0 | Rob Tuytel |
| `T_brown_mud_02_*`, `MI_DC_Mud` | `CC0/polyhaven/brown_mud_02` | CC0 | Rob Tuytel |
| `T_chipped_concrete_*`, `MI_DC_Concrete` | `CC0/polyhaven/chipped_concrete` | CC0 | Amal Kumar |
| `T_blue_plaster_weathered_*`, `MI_DC_Plaster` | `CC0/polyhaven/blue_plaster_weathered` | CC0 | Amal Kumar |
| `T_CorrugatedSteel009_*`, `MI_DC_Steel` | `CC0/ambientcg/CorrugatedSteel009` | CC0 | ambientCG |

URLs are in each folder's `source.json` under `C:\FO5_AssetLibrary\CC0`. Unpacked sets also import an AO map, and the steel set imports a metalness map. Those maps are in `/Game/Art` and are not sampled yet: unpacked instances use roughness plus a metallic scalar (`MI_DC_Steel` is 1).
