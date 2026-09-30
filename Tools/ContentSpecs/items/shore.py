"""Item definitions for the Authority Shore (Phases 1-5): the boathouse, Shore Watch, the Survey Launch.

Moved unchanged from create_items.py in Phase 6 (VS-02). Item ids are stored in saves: never rename one.
Owner: the Integrator (accepted content). Spec shape: see Tools/ContentSpecs/README.md.
"""

# size_cm: for placeholder meshes, the item's real-world size; the mesh is scaled to fit.
# fit_cm: for an imported prop, the longest side. The scale is uniform so the mesh keeps its shape.
# None keeps the mesh at its authored scale.
CRATE = "/Game/Art/PolyHaven/wooden_crate_01/wooden_crate_01_2k/StaticMeshes/wooden_crate_01"
ITEMS = [
    dict(asset="DA_Item_Ammo9mm", item_id="ammo_9mm", name="9mm Rounds",
         description="Loose pistol rounds, some hand-reloaded.",
         category="Item.Ammo", weight=0.01, value=1, stack=999,
         mesh=CRATE, size_cm=None, fit_cm=14),
    dict(asset="DA_Item_Pistol", item_id="pistol_service", name="Service Pistol",
         description="A pre-collapse sidearm. Worn, but it still cycles.",
         category="Item.Weapon.Firearm", weight=1.2, value=120, stack=1,
         mesh="/Game/Weapons/Pistol/Meshes/SM_Pistol", size_cm=None,
         firearm=dict(ammo="/Game/Items/DA_Item_Ammo9mm", mag=15, damage=25.0,
                      range=10000.0, fire_interval=0.18, reload=1.3,
                      recoil_pitch=1.4, recoil_yaw=0.4, recoil_recovery=12.0,
                      equip_offset=(38.0, 12.0, -20.0), equip_rot=(6.0, -90.0, 4.0),
                      fire_sound="/Game/Audio/Weapons/S_DC_PistolShot",
                      dry_fire_sound="/Game/Audio/Weapons/S_DC_PistolDry")),
    dict(asset="DA_Item_FieldDressing", item_id="field_dressing", name="Field Dressing",
         description="Boiled cloth and a strip of tape. Stops bleeding, mostly.",
         category="Item.Consumable.Medical", weight=0.1, value=15, stack=10,
         mesh="/Game/Art/Meshy/field_dressing/SM_field_dressing", size_cm=None, fit_cm=10),
    dict(asset="DA_Item_SalvagedWiring", item_id="salvage_wiring", name="Salvaged Wiring",
         description="Copper wire stripped from dead machinery.",
         category="Item.Salvage", weight=0.25, value=4, stack=50,
         mesh="/Game/LevelPrototyping/Meshes/SM_Cylinder", size_cm=(16, 16, 5)),
    dict(asset="DA_Item_RadioCoil", item_id="radio_coil", name="Relay Coil",
         description="A hand-wound copper coil from a Maritime Authority relay. It is warm, and it hums when you hold it close.",
         category="Item.Quest", weight=0.2, value=8, stack=1,
         mesh="/Game/Art/Meshy/radio_coil/SM_radio_coil", size_cm=None, fit_cm=12),
    dict(asset="DA_Item_SurveyChart", item_id="survey_chart", name="Sounder Chart",
         description="A roll of the Tern's depth-sounder paper, torn off at the mark. Regular spikes, evenly spaced, and someone has pencilled AGAIN beside the last one.",
         category="Item.Quest", weight=0.05, value=6, stack=1,
         mesh="/Game/Art/Meshy/sounder_chart/SM_sounder_chart", size_cm=None, fit_cm=18),
]
