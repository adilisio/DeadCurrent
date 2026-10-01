# Great Lakes Working Settlement Kit

Phase 6, VS-05 (WP-KIT). Tier B: buildings are composed from this vocabulary. None of them is its own mesh.

Catalog data lives in `Tools/Kits/great_lakes_settlement/kit.json`. Module geometry lives in `Tools/EditorScripts/kit/modules.py` and is imported by `Tools/EditorScripts/import_kit_settlement.py`. Compositions live in `Tools/Kits/great_lakes_settlement/structures/`. The gym that places three of them is `Tools/EditorScripts/kit/build_kit_gym.py` (`Lvl_KitGym`, a development map).

No Meshy credits. No unique generated or hand-modelled building.

## What a structure is

A composition is one JSON file. `compose.plan(spec, kit)` turns it into placements with no engine and no randomness. `compose.place(spec, kit, location, yaw)` spawns them.

The local frame is centimetres. The origin is the front-left corner at ground level. +X runs along the front, +Y toward the back, +Z up. The front faces -Y. `place()` rotates that frame by `yaw` about the origin, then moves it to `location`.

```json
{
  "id": "kit_store",
  "footprint": [6, 8],
  "storeys": 2,
  "base": 40,
  "skins": {"walls": "RedBoard", "roof": "ShingleRoof"},
  "side_skins": {"back": "BoardGrey"},
  "open_sides": ["front"],
  "openings": {"front": [{"type": "door", "at": 2, "storey": 1}, {"type": "window", "at": 0}]},
  "roof": {"type": "gable", "ridge": "y", "pitch": 35, "overhang": 40},
  "porch": {"side": "front", "depth": 2.0, "from": 0, "to": 6},
  "stair": {"side": "left", "to_storey": 2},
  "chimney": [3.9, 4.6],
  "attachments": [{"piece": "Barrel", "at": [0.6, 1.2, 0.4], "yaw": 0}],
  "gym": false
}
```

| Field | Meaning |
| --- | --- |
| `id` | Structure id. Placed actors are tagged `Structure:<id>`. |
| `footprint` | `[width, depth]` in metres. Width is the front. |
| `storeys` | 1 or 2. A storey is 2.8 m. |
| `base` | Foundation height in centimetres. Above 0, the floor sits on pilings. |
| `skins` | Slot defaults for this structure. Missing keys use `kit.json` `default_skins`. |
| `side_skins` | Optional wall skin per side (`front`, `right`, `back`, `left`). |
| `open_sides` | Ground-floor sides with no wall. Corner boards become posts; a beam closes the top. |
| `openings` | Per side, `{type: door\|window, at: metres from the side's start, storey: 1}`. An opening occupies 2 m. |
| `roof` | `gable` (ridge `x` or `y`), `lean_to` (high at the back), or `flat`. Optional `pitch` (degrees) and `overhang` (cm). |
| `porch` | Front only, in this version. `depth` in metres, optional `from` / `to` in metres along the front. |
| `stair` | An outside stair to an upper door. The side must be `left` or `right` and that side must have a door on `to_storey`. |
| `chimney` | `[x, y]` in metres. |
| `attachments` | Library props. `at` is metres in the structure frame. |
| `gym` | `true` places it on `Lvl_KitGym`. |

Walls are filled with 4 m, then 2 m, then 1 m solid panels around the 2 m openings. A gap smaller than 1 m is an error.

## Compose API

```python
import compose
kit = compose.load_kit()                         # Tools/Kits/great_lakes_settlement/kit.json
spec = compose.load_structure("store")           # structures/<name>.json
placements = compose.plan(spec, kit)             # pure Python, deterministic
actor = compose.place(spec, kit, (x, y, z), yaw, folder="KitGym")
```

`plan()` returns `Placement` objects: piece id, local location (cm), rotation (pitch, yaw, roll), scale, and slot-to-skin. The same spec always returns the same list.

`place()` spawns one actor labelled `Structure_<id>`, tagged `Structure:<id>`. Kit modules are instanced: one `InstancedStaticMeshComponent` per (piece, skin set), tagged `Kit:<PieceId>` and `Structure:<id>`. Library attachments are their own `StaticMeshActor`s (pack materials are not flagged for instancing), tagged `Kit:<PieceId>` and `Structure:<id>`, attached to the structure, seated on the given height.

Later settlement scripts import `compose` the same way. They do not duplicate the placement math.

## Modules

Authored once, imported as `/Game/World/Kits/GreatLakesSettlement/Meshes/SM_Kit_<Id>`. The local frame matches the structure frame: origin at the outer, left, bottom corner; +X along the piece; +Y is thickness (a wall's inside); +Z up.

Scalable modules are 1 m units stretched by `compose()`. Surfaces are triplanar in world space, so the stretch does not stretch the texture. Walls, the door, the window, the stair, the railing, and the chimney are never scaled.

Collision is complex-as-simple on every module. The gym only needs walkable decks and stairs; the same meshes block where a player will meet a wall, post, rail, roof, or chimney.

| Id | What it is | Scale axes | Collision | Skins |
| --- | --- | --- | --- | --- |
| `Wall_100` | Solid wall, 1 × 2.8 m, 15 cm thick | none | blocks | `skin` |
| `Wall_200` | Solid wall, 2 × 2.8 m | none | blocks | `skin` |
| `Wall_400` | Solid wall, 4 × 2.8 m | none | blocks | `skin` |
| `Wall_200_Door` | 2 m wall, closed door 1.0 × 2.1 m, trim frame | none | blocks | `skin`, `trim`, `door` |
| `Wall_200_Window` | 2 m wall, window 0.9 × 0.9 m at 1.0 m, trim frame | none | blocks | `skin`, `trim`, `pane` |
| `Gable_100` | Isosceles gable, unit 1 m span and rise | X, Z | blocks | `skin` |
| `Rake_100` | Lean-to end, unit right triangle | X, Z | blocks | `skin` |
| `Roof_100` | Roof panel, 1 × 1 m, 12 cm | X, Y | blocks | `roof`, `underside` |
| `Ridge_100` | Ridge cap, 1 m | X | blocks | `trim`, `underside` |
| `Deck_100` | Floor, porch, or landing, 1 × 1 m, 20 cm, top at Z 0 | X, Y | walkable | `deck`, `underside` |
| `Post_100` | Post or piling, 16 cm square, 1 m | Z | blocks | `trim` |
| `Beam_100` | Beam or fascia, 16 cm square, 1 m | X | blocks | `trim`, `underside` |
| `Corner_280` | Corner board, 20 cm square, one storey | Z | blocks | `trim` |
| `Stair_280` | Outside stair, 1 m wide, 14 steps of 20 cm over 4.2 m, ascending +Y | none | walkable | `deck` |
| `Railing_200` | 2 m rail: three posts and a top rail | none | blocks | `trim` |
| `Chimney_200` | Stack, 60 cm square, 2 m | none | blocks | `masonry` |

16 modules. The slice ceiling is 30.

Source and license for every module mesh: project-authored geometry in `modules.py`, imported through `import_kit_settlement.py`. Not from a pack, not generated.

## Skins

Material instances under `/Game/World/Kits/GreatLakesSettlement/Materials/`. Names are `MI_DC_Kit_*`. Triplanar instances of `M_DC_Surface` where the kit has its own textures; tinted children of the shore surfaces otherwise; flat colour for glass and for downward faces (the triplanar master lights a downward face as if it faced up, the same limit VS-04 recorded).

`M_DC_Surface` is flagged `used_with_instanced_static_meshes`. Without that flag an instanced wall renders as the engine checker. The flat master `M_DC_Kit_Flat` carries the same flag. Child instances of the shore surfaces inherit it from `M_DC_Surface`.

| Skin id | Asset | Reads as | Source |
| --- | --- | --- | --- |
| `GreyShingle` | `MI_DC_Kit_GreyShingle` | Weathered grey wood shingles | ambientCG `WoodSiding011`, CC0 1.0, 2K. Tile 220 cm, tint slightly down. |
| `ShingleRoof` | `MI_DC_Kit_ShingleRoof` | The same shingles, darkened, as a roof | `WoodSiding011` again. Tile 150 cm. This is a wood-shingle roof, not asphalt and not clay tile. |
| `BoardGrey` | `MI_DC_Kit_BoardGrey` | Light weathered boards | ambientCG `Planks012`, CC0 1.0, 2K, tinted (0.80, 0.80, 0.80). |
| `Tarred` | `MI_DC_Kit_Tarred` | Tarred timber | `Planks012`, tinted near black (0.17, 0.16, 0.15). Not a tar-paper scan. |
| `RedBoard` | `MI_DC_Kit_RedBoard` | Faded red horizontal siding | ambientCG `WoodSiding005`, CC0 1.0, 2K. |
| `Corrugated` | `MI_DC_Kit_Corrugated` | Cold corrugated steel | Child of `MI_DC_Steel` (ambientCG `CorrugatedSteel009`, already in the project). |
| `RustTrim` | `MI_DC_Kit_RustTrim` | Rust-paint trim | Child of `MI_DC_RustPaint`. |
| `Whitewash` | `MI_DC_Kit_Whitewash` | Whitewash | Child of `MI_DC_Plaster`, lifted. |
| `Concrete` | `MI_DC_Kit_Concrete` | Concrete | Child of `MI_DC_Concrete`. |
| `Underside` | `MI_DC_Kit_Underside` | Dark flat soffit | Authored flat colour. |
| `Pane` | `MI_DC_Kit_Pane` | Dark glass | Authored flat colour. |

`WoodSiding011` has color, displacement, and a DirectX normal, and no roughness map. The two shingle skins sample `Planks012` roughness. Recorded in each folder's `source.json` under `C:\FO5_AssetLibrary\CC0\ambientcg\`.

Not used, on purpose:

- ambientCG `Bitumen`: no such asset. The survey's tar-paper claim was wrong.
- ambientCG `RoofingTiles014`: clay tile, not asphalt shingles.
- Poly Haven `Aerial Asphalt 01`: not used. It would only be an art-direction stand-in for tar paper, and the kit uses tarred boards and corrugated sheet instead.
- Poly Haven `roof_slates_02`: slate. Left out; shingle and corrugated fit these buildings better.

## Attachments

Library props. They are not structural modules. `report_kit_usage.py` counts them as kit pieces and not as bespoke structure.

| Id | Mesh | Source | License |
| --- | --- | --- | --- |
| `Barrel` | `/Game/Smugglers_cove/meshes/props/SM_wooden_barrels_01_a` | Smugglers Cove pack | owned |
| `BarrelStack` | `SM_wooden_barrels_01_b` | same | owned |
| `Bucket` | `SM_wooden_bucket_01` | same | owned |
| `CrateSlat` | `SM_wooden_crate_02` | same | owned |
| `Crate` | `/Game/Art/PolyHaven/wooden_crate_01/...` | Poly Haven `wooden_crate_01`, already in the project | CC0 |
| `Lantern` | `/Game/Art/PolyHaven/Lantern_01/...` | Poly Haven `Lantern_01`, already in the project | CC0 |
| `Pole` | `/Game/Smugglers_cove/meshes/structures/SM_wooden_pier_poles` | Smugglers Cove, already in the project | owned |

The new Smugglers Cove props were migrated with `Tools\ImportPackAssets.ps1`. Their source textures are 4096; the committed textures are 1024. Provenance rows are in `Design/art_pipeline.md`.

Not in v1, and not required to prove composition: junkyard drums and pallets, electrical boxes, shutters, a ladder, rope, floats, net racks, a skiff. The pier pole is available as `Pole` when a composition wants the pack mesh; pilings in the grammar are the scalable `Post_100`.

## The three gym structures

They are different buildings, not three skins on one box.

| | Store `kit_store` | Salvage shed `kit_salvage_shed` | Cottage `kit_cottage` |
| --- | --- | --- | --- |
| Like | Marthe's store, loft stair, gable to the street | The Pruitts' open shed | Odette's cottage |
| Footprint | 6 × 8 m | 6 × 4 m | 5 × 6 m |
| Storeys | 2, on a 40 cm piling base | 1, on the ground | 1, on a 30 cm base |
| Walls | Closed. Faded red siding, grey board on the back. | Front open. Corrugated sheet. | Closed. Grey wood shingles. |
| Roof | Gable, ridge across the depth, so the gable faces the street. Shingle. | Lean-to, high at the back. Corrugated. | Gable, ridge along the front, so the eaves face the street. Shingle. |
| Extra | Full-width porch. Outside stair to the loft door on the left. | Posts and a beam on the open front. | Short porch. Chimney. |
| Props | Barrel, slat crate, barrel stack, lantern | Two crates, barrel stack, slat crate, bucket | Bucket, lantern |

`kit/verify_kit.py` fails if any two of these share footprint, storey count, roof type, open sides, porch, stair, and chimney together.

## Compositions for the rest of the slice

These are data only. The gym does not place them. The settlement cells will.

| File | Id | What it is |
| --- | --- | --- |
| `smokehouse.json` | `kit_smokehouse` | 3 × 3 m, tarred board, corrugated gable, one door, a stack. |
| `harbor_shed.json` | `kit_harbor_shed` | 4 × 6 m, grey board, corrugated gable to the front. Place it twice for the two harbor sheds. |
| `cable_hut.json` | `kit_cable_hut` | 3 × 3 m, concrete, flat roof, a steel-looking door. |
| `net_loft_shell.json` | `kit_net_loft_shell` | 6 × 8 m interior shell, one storey, board walls, gable seen from inside. The loft above the store, as a room. |

The same grammar covers the structures that do not have their own file yet. A settlement cell adds a JSON; it does not add a mesh.

| Slice structure | How the kit builds it |
| --- | --- |
| Marthe's store and the loft stair | `store.json` (in the gym now). |
| Net loft interior | `net_loft_shell.json`. |
| Odette's cottage | `cottage.json` (in the gym now). |
| Leclair house front and porch | The cottage grammar: one storey, gable, a front porch, whitewash trim, a different footprint. |
| Pruitts' salvage shed | `salvage_shed.json` (in the gym now). |
| Smokehouse | `smokehouse.json`. |
| Two harbor sheds | `harbor_shed.json`, placed twice. |
| Cable hut | `cable_hut.json`. |
| Tower base room | The cable-hut grammar at a larger footprint: concrete walls, a door, a flat roof. The lighthouse tower above it is not a kit building. |
| *Ida* deck furniture and wheelhouse front | Wall, door, window, and deck modules, plus the attachments. The hull is not a kit building. |
| Hale's cutter deck furniture | The same modules and attachments. The hull is not a kit building. |

## What this kit does not cover

Vessels (hulls), the lighthouse tower shell, and the *Grey*. Those stay bespoke, and they are not buildings. The kit also does not place Gameplay, doors that open, interiors beyond the net-loft shell, or anything on `Lvl_PointeSombre`.

## How to add a module

1. Add the geometry in `modules.py` (`MODULES`, and `SCALABLE` if `compose()` may stretch it, and a `COLLISION` note).
2. Give it material slot names the skins already understand (`skin`, `trim`, `door`, `pane`, `roof`, `underside`, `deck`, `masonry`).
3. Run `Tools\RebuildContent.bat import_kit_settlement`. An unchanged module is skipped; a changed one is replaced.
4. Add a row to this catalog, including source and license. A new texture needs a `source.json` under `C:\FO5_AssetLibrary\CC0\` and a provenance row in `Design/art_pipeline.md`.
5. If it is a library prop instead of a module, add it under `attachments` in `kit.json` and migrate the pack asset with `Tools\ImportPackAssets.ps1` (1K textures).

Stay at or under 30 distinct modules.

## Checks

- `Tools\RebuildContent.bat import_kit_settlement` then `kit\build_kit_gym` then `kit\verify_kit`.
- `Tools\RebuildContent.bat report_kit_usage` writes `Saved/KitUsage/Lvl_KitGym.txt`. The recorded copy is `Design/Kits/kit_usage_report.md`.
- `Tools\ReviewCapture.bat Lvl_KitGym`.
- `Lvl_KitGym` is a development map in `Tools/Maps.bat`. It is not in the production list, so it is not cooked and not part of the production test loop. The `.umap` is a build product and is not committed on `vs/kit`.
