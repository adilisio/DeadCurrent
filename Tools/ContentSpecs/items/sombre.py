"""Item definitions for Pointe Sombre, the Phase 6 slice ("The Wrong Characteristic").

Ids are the ledger's (`Design/POIs/sombre_ids.md` §4): never rename one, and request a new one from the Integrator.
Item ids carry no area prefix, as shipped (`ammo_9mm`); `sombre_vault_key` keeps the dialogue's spelling.
The slice's loot reuses the shipped `ammo_9mm`, `field_dressing`, and `salvage_wiring` (`items/shore.py`); nothing
here redefines them. Every slice item is an `Item.Quest` object: evidence, a key, or a part for the light.
Texts come from the slice documents where they exist (`Design/Narrative/SLICE_WRONG_CHARACTERISTIC.md` B4,
`PROLOGUE_POSTED_FROM_THE_SHORE.md` P1); the rest are short, plain descriptions and decide no lore.
World meshes are existing project props or primitives: the evidence is picked up from inspectables (plan §8.2 #8),
so these meshes show only if an item is dropped.
Owner: WP-NARR (VS-09). Spec shape: see Tools/ContentSpecs/README.md.
"""

CUBE = "/Game/LevelPrototyping/Meshes/SM_Cube"
CYLINDER = "/Game/LevelPrototyping/Meshes/SM_Cylinder"
LANTERN = "/Game/Art/PolyHaven/Lantern_01/Lantern_01_2k/StaticMeshes/Lantern_01"
CAN = "/Game/Art/PolyHaven/can_rusted/can_rusted_2k/StaticMeshes/can_rusted_2k"
CHART = "/Game/Art/Meshy/sounder_chart/SM_sounder_chart"

ITEMS = [
    # The crossing: the recap, and the condition on Odette's tie key.
    dict(asset="DA_Item_LivLetter", item_id="liv_letter", name="Liv's Letter",
         description="Liv's letter. You know it by heart. Her capital A has no crossbar; she always said a crossbar "
                     "reads as a dash in a transcript.",
         category="Item.Quest", weight=0.01, value=0, stack=1,
         mesh=CUBE, size_cm=(21, 15, 0.3)),

    # The vault: evidence, and the copy of her chart.
    dict(asset="DA_Item_LivNote", item_id="liv_note", name="Liv's Note",
         description="Odette — I took the card. Your light was about to join them again and I can't let it. "
                     "Light it by hand. Oil, the old clockwork, your own two arms. I'm sorry for what I'm asking. "
                     "Don't let anyone put it back on the Line. Not the Compact. Not anybody. — L.",
         category="Item.Quest", weight=0.01, value=0, stack=1,
         mesh=CUBE, size_cm=(15, 10, 0.3)),
    dict(asset="DA_Item_LivChart", item_id="liv_chart", name="Copy of Liv's Chart",
         description="Your copy of the chart pinned in the vault. A pencil line runs east from the Authority Shore, "
                     "a fainter one from Pointe Sombre, and where they cross, a long thin question mark.",
         category="Item.Quest", weight=0.05, value=0, stack=1,
         mesh=CHART, size_cm=None, fit_cm=18),
    dict(asset="DA_Item_VaultAccessLog", item_id="vault_access_log", name="Vault Sign-in Board",
         description="Two entries in chalk, four months old: O.B. and L.K. Nothing since.",
         category="Item.Quest", weight=0.4, value=0, stack=1,
         mesh=CUBE, size_cm=(30, 20, 1.5)),

    # The lamp room: the cut feed.
    dict(asset="DA_Item_CutCableEnd", item_id="cut_cable_end", name="Cut Cable End",
         description="A length of the lamp's feed cable. The end is cut clean. Shears, not a saw.",
         category="Item.Quest", weight=0.3, value=0, stack=1,
         mesh=CYLINDER, size_cm=(4, 4, 25)),

    # The west headland: the false light.
    dict(asset="DA_Item_FalseLantern", item_id="false_lantern", name="Storm Lantern",
         description="The lantern from the post on the west head. The glass is sooted on the lakeward side.",
         category="Item.Quest", weight=1.0, value=0, stack=1,
         mesh=LANTERN, size_cm=None, fit_cm=30),

    # Hale: the Compact's card for the node panel.
    dict(asset="DA_Item_SectionKeyCompact", item_id="section_key_compact", name="Compact Section Card",
         description="Warden Hale's card for the tower's panel. Seated, it puts the light back on the Line.",
         category="Item.Quest", weight=0.01, value=0, stack=1,
         mesh=CUBE, size_cm=(8.5, 5.4, 0.3)),

    # The hand light: a part for the clockwork, and fuel for the burner.
    dict(asset="DA_Item_ClockworkPawl", item_id="clockwork_pawl", name="Winch Pawl",
         description="A steel pawl from a laker's winch. The lamp room's clockwork takes the same part.",
         category="Item.Quest", weight=0.3, value=0, stack=1,
         mesh=CUBE, size_cm=(7, 2, 1.5)),
    dict(asset="DA_Item_LampOil", item_id="lamp_oil", name="Can of Lamp Oil",
         description="A can of oil, enough to fill the lamp room's old burner.",
         category="Item.Quest", weight=1.5, value=0, stack=1,
         mesh=CAN, size_cm=None, fit_cm=20),

    # Odette: the hatch key. Not removed when used; the hatch keeps working by sombre.vault_opened.
    dict(asset="DA_Item_SombreVaultKey", item_id="sombre_vault_key", name="Vault Hatch Key",
         description="The keeper's key to the hatch at the foot of the tower.",
         category="Item.Quest", weight=0.05, value=0, stack=1,
         mesh=CYLINDER, size_cm=(1.2, 1.2, 9)),
]
