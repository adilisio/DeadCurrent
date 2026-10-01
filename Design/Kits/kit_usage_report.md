# Settlement kit usage

Recorded from `Tools\RebuildContent.bat report_kit_usage` on 2026-10-01, after the second identical run. The two report files hashed the same (`252C37C7485E6311C15DE56E6CD9DA8D5303078E34A948FFBE53BA337868BB48`). Source text: `Saved/KitUsage/Lvl_KitGym.txt` (that folder is not committed).

16 authored modules and 7 library attachments. No bespoke building mesh.

## The gym (three placed structures)

| Measure | Result | Target | Met for the gym? |
| --- | --- | --- | --- |
| Structures composed from the kit | 3 | ≥ 10 once the slice is placed | The gym target is 3. The slice target is later. |
| Distinct kit pieces used | 22 | ≤ 30 | Yes |
| Pieces reused in ≥ 3 structures | 8 (`Beam_100`, `Corner_280`, `Deck_100`, `Post_100`, `Roof_100`, `Wall_100`, `Wall_200`, `Wall_200_Window`) | ≥ 10 once the slice is placed | The real number for these three is 8. |
| Share of structural placements that are kit pieces | 100.0% (109 of 109) | ≥ 75% | Yes |
| Bespoke structural assets (buildings) | 0 | 0 | Yes |

| Structure | Placements |
| --- | --- |
| `kit_cottage` | 39 |
| `kit_salvage_shed` | 22 |
| `kit_store` | 59 |

They are not three skins on one box. The store is 6 × 8 m, two storeys, a street gable, a porch, and an outside stair. The shed is 6 × 4 m, one storey, an open front, and a lean-to. The cottage is 5 × 6 m, one storey, eaves to the front, a short porch, and a chimney. `verify_kit.py` fails if that signature matches.

## Projection (all seven compositions, not yet on the island)

| Measure | Result | Target | Met? |
| --- | --- | --- | --- |
| Structures composed from the kit | 7 | ≥ 10 | Not yet. The other slice structures are the same grammar, described in the catalog, and are not separate files yet (Leclair front, tower base room, two decks). Two harbor sheds are one file placed twice. |
| Distinct kit pieces used | 22 | ≤ 30 | Yes |
| Pieces reused in ≥ 3 structures | 12 | ≥ 10 | Yes, on the seven compositions. |
| Share of structural placements that are kit pieces | 100.0% (187 of 187) | ≥ 75% | Yes |
| Bespoke structural assets (buildings) | 0 | 0 | Yes |

The twelve reused pieces are `Beam_100`, `Corner_280`, `Deck_100`, `Gable_100`, `Post_100`, `Ridge_100`, `Roof_100`, `Wall_100`, `Wall_200`, `Wall_200_Door`, `Wall_200_Window`, `Wall_400`.

| Structure | Placements | Where |
| --- | --- | --- |
| `kit_cable_hut` | 14 | composition only |
| `kit_cottage` | 39 | gym |
| `kit_harbor_shed` | 19 | composition only (place twice) |
| `kit_net_loft_shell` | 26 | composition only |
| `kit_salvage_shed` | 22 | gym |
| `kit_smokehouse` | 19 | composition only |
| `kit_store` | 59 | gym |
