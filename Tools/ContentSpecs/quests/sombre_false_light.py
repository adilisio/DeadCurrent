"""False Light (sombre.false_light): who showed the lantern on the west head the night the Ida hit.

Stages are the beat script's §6 (`Design/Narrative/SLICE_WRONG_CHARACTERISTIC.md`), with the ledger's ids
(`Design/POIs/sombre_ids.md` §3, §5). Quest and stage ids are stored in saves: never rename one.

  asked --(sombre.headland discovered)--> found --(dell_confessed)----------> named --(exposed_pruitts)--> done
                                                --(carrying false_lantern)-->       --(varga_told)------> done

- Two approaches only (plan §8.2 #2, #3): the evidence approach (take the lantern; no build needed) and the
  confession approach (Dell, by `[Persuasion 2]` or `[Survival 2]`). There is no "catch them at it" stage and no
  force branch: catching Dell at the post in the first storm is the same conversation and the same confession.
- Closed by telling Varga a name (`sombre.varga_told`) or by giving the lantern at the net loft
  (`sombre.exposed_pruitts`). Offering Dell the keeper's post is an outcome in his dialogue, not a stage.
- `done` sets `sombre.false_light_taken`, so the lantern never burns again however the quest was closed.
- Started only by Varga's first conversation, beside `sombre.characteristic`.
Owner: WP-NARR (VS-09). Spec shape: see Tools/ContentSpecs/README.md.
"""


def flag(name):
    return cond("WORLD_FLAG", id=name)


SOMBRE_FALSE_LIGHT = dict(
    asset="DA_Quest_SombreFalseLight",
    quest_id="sombre.false_light",
    name="False Light",
    start="asked",
    stages=[
        dict(
            id="asked",
            objective="Someone showed a light on the west headland. Varga wants a name.",
            transitions=[
                dict(next="found", conditions=[cond("LOCATION_DISCOVERED", id="sombre.headland")]),
            ],
        ),
        dict(
            id="found",
            objective="The lantern post is in a hide on the west head. Find who lights it.",
            transitions=[
                dict(next="named", conditions=[flag("sombre.dell_confessed")]),
                dict(next="named", conditions=[cond("HAS_ITEM", id="false_lantern")]),
            ],
        ),
        dict(
            id="named",
            objective="Tell Varga, or bring the lantern to the net loft.",
            transitions=[
                dict(next="done", conditions=[flag("sombre.exposed_pruitts")]),
                dict(next="done", conditions=[flag("sombre.varga_told")]),
            ],
        ),
        dict(
            id="done",
            completes=True,
            objective="The false light won't burn again.",
            on_enter=[cons("SET_WORLD_FLAG", id="sombre.false_light_taken")],
        ),
    ],
)

QUESTS = [SOMBRE_FALSE_LIGHT]
