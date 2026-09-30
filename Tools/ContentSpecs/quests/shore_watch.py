"""Shore Watch (shore.watch): Mara wants the scavenger's rigged relay silenced.

Moved unchanged from create_quest.py in Phase 6 (VS-02). Quest ids and stage ids are stored in saves: never rename one.
Owner: the Integrator (accepted content). Spec shape: see Tools/ContentSpecs/README.md.
"""

# Shore Watch: Mara wants the scavenger's rigged relay silenced.
#   accepted --(boat.scavenger dead)--> return_killed --(Mara)--> done_killed   flag shore.path_cleared
#            --(carrying radio_coil)--> return_coil   --(Mara)--> done_coil     flag shore.relay_recovered
# Mara's dialogue moves the return stages to the outcome stages and hands out the rewards.
SHORE_WATCH = dict(
    asset="DA_Quest_ShoreWatch",
    quest_id="shore.watch",
    name="Shore Watch",
    start="accepted",
    stages=[
        dict(
            id="accepted",
            objective="Silence the relay at the scavenger's camp: kill him, or pull the coil from his rig without a fight.",
            transitions=[
                dict(next="return_killed", conditions=[cond("ACTOR_DEAD", id="boat.scavenger")]),
                dict(next="return_coil", conditions=[cond("HAS_ITEM", id="radio_coil")]),
            ],
        ),
        dict(
            id="return_killed",
            objective="The scavenger is dead. Tell Mara the relay has no one to tend it.",
        ),
        dict(
            id="return_coil",
            objective="You have the relay coil. Bring it to Mara.",
        ),
        dict(
            id="done_killed",
            completes=True,
            objective="You killed the scavenger. The shore path is clear and his relay has gone cold.",
            on_enter=[cons("SET_WORLD_FLAG", id="shore.path_cleared")],
        ),
        dict(
            id="done_coil",
            completes=True,
            objective="You took the relay coil without a fight. Mara is listening to it. The scavenger still walks the shore.",
            on_enter=[cons("SET_WORLD_FLAG", id="shore.relay_recovered")],
        ),
    ],
)

QUESTS = [SHORE_WATCH]
