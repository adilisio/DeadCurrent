"""Loader and helpers for per-file content specs (Phase 6, VS-02). Integrator-owned.

create_items.py, create_quest.py, and create_dialogue.py no longer hold content. Each loads every
`Tools/ContentSpecs/<kind>/*.py` file (sorted by file name; names starting with `_` are skipped) and writes the assets
those files describe. One spec file has one owner, so a builder adds `dialogue/sombre_varga.py` without touching Mara's.
The contract and an example of each kind are in `Tools/ContentSpecs/README.md`.

A spec file is a plain Python file. It is run with these names already defined, so it needs no imports:
  cond(type, id=None, stage=None, quantity=1, negate=False)   a rule condition (the shared rule language)
  cons(type, id=None, stage=None, quantity=1)                  a rule consequence; GIVE_ITEM / REMOVE_ITEM of an item
                                                               defined in any items spec set the asset reference too
  COND, CONS                                                   the DCConditionType / DCConsequenceType enums
  unreal
and it must define one list: ITEMS (items), QUESTS (quests), or DIALOGUES (dialogue), of dicts in the shape the matching
create_*.py documents. Asset names and ids must be unique across every file of a kind; a clash stops the script and names
both files. A spec may refer to assets from files that sort before its own (items are generated before quests, and
quests before dialogue, by Tools\\RebuildContent.bat).

Nothing here decides content. Changing how a spec is turned into an asset is the Integrator's change, and must pass
`Tools\\DumpContent.bat` before and after with an empty diff.
"""
import os
import runpy

import unreal

COND = unreal.DCConditionType
CONS = unreal.DCConsequenceType

ITEMS_FOLDER = "/Game/Items"
_item_paths = None


def spec_root():
    return os.path.join(unreal.SystemLibrary.get_project_directory(), "Tools", "ContentSpecs")


def cond(type_name, id=None, stage=None, quantity=1, negate=False):
    c = unreal.DCGameplayCondition()
    c.set_editor_property("type", getattr(COND, type_name))
    c.set_editor_property("id", unreal.Name(id) if id else unreal.Name())
    c.set_editor_property("stage", unreal.Name(stage) if stage else unreal.Name())
    c.set_editor_property("quantity", quantity)
    c.set_editor_property("negate", negate)
    return c


def item_paths():
    """{item_id: asset path} for every item in every items spec file."""
    global _item_paths
    if _item_paths is None:
        _item_paths = {spec["item_id"]: f"{ITEMS_FOLDER}/{spec['asset']}" for spec in load_specs("items", "ITEMS")}
    return _item_paths


def cons(type_name, id=None, stage=None, quantity=1):
    c = unreal.DCGameplayConsequence()
    c.set_editor_property("type", getattr(CONS, type_name))
    c.set_editor_property("id", unreal.Name(id) if id else unreal.Name())
    c.set_editor_property("stage", unreal.Name(stage) if stage else unreal.Name())
    c.set_editor_property("quantity", quantity)
    if type_name in ("GIVE_ITEM", "REMOVE_ITEM") and id in item_paths():
        c.set_editor_property("item", unreal.load_asset(item_paths()[id]))
    return c


# Which key of each kind's spec dicts must be unique across files.
_UNIQUE_KEYS = {"items": ("asset", "item_id"), "quests": ("asset", "quest_id"), "dialogue": ("asset", "dialogue_id")}


def spec_files(kind):
    folder = os.path.join(spec_root(), kind)
    if not os.path.isdir(folder):
        raise RuntimeError(f"No spec folder {folder}")
    return [os.path.join(folder, name) for name in sorted(os.listdir(folder))
            if name.endswith(".py") and not name.startswith("_")]


def load_specs(kind, list_name):
    """Every spec dict from Tools/ContentSpecs/<kind>/*.py, in file-name order, checked for duplicate assets and ids."""
    helpers = dict(cond=cond, cons=cons, COND=COND, CONS=CONS, unreal=unreal)
    specs = []
    seen = {}
    for path in spec_files(kind):
        module = runpy.run_path(path, init_globals=helpers)
        found = module.get(list_name)
        if not isinstance(found, list):
            raise RuntimeError(f"{path} must define {list_name} as a list")
        for spec in found:
            for key in _UNIQUE_KEYS[kind]:
                marker = (key, spec[key])
                if marker in seen:
                    raise RuntimeError(f"{key} '{spec[key]}' is defined in both {seen[marker]} and {path}")
                seen[marker] = path
            specs.append(spec)
    return specs
