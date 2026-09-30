# Content specs

Items, quests, and dialogue are data. Each lives in **its own file** here, and one file has one owner. The generators `Tools/EditorScripts/create_items.py`, `create_quest.py`, and `create_dialogue.py` contain no content: they load every spec file in the matching folder and write the Data Assets. A builder adds `dialogue/sombre_varga.py` without touching Mara's conversation, and two builders never edit the same file.

```
Tools/ContentSpecs/
  items/     *.py   defines ITEMS      -> /Game/Items/<asset>
  quests/    *.py   defines QUESTS     -> /Game/Quests/<asset>
  dialogue/  *.py   defines DIALOGUES  -> /Game/Dialogue/<asset>
```

## Rules

- **One list per file**: `ITEMS`, `QUESTS`, or `DIALOGUES`, of dicts. The dict shapes are the ones the accepted files use (`items/shore.py`, `quests/shore_watch.py`, `dialogue/mara_intro.py` are the reference examples). A builder does not invent new keys; a new key needs the matching `create_*.py` change, which is the Integrator's.
- **No imports needed.** A spec file is run with `cond`, `cons`, `COND`, `CONS`, and `unreal` already defined (`Tools/EditorScripts/content_specs.py`).
  - `cond("WORLD_FLAG", id="sombre.storm", negate=True)` is a rule condition; `cons("SET_WORLD_FLAG", id="sombre.storm")` a consequence. The type names are the `DCConditionType` / `DCConsequenceType` members (see `Design/technical_architecture.md`, "Rules").
  - `cons("GIVE_ITEM", id="radio_coil")` and `REMOVE_ITEM` also set the asset reference when the id belongs to an item in any `items/` file.
- **Order.** Files load in file-name order; `_`-prefixed files are skipped (use `_scratch.py` for notes or drafts). `Tools\RebuildContent.bat` generates items, then quests, then dialogue, so dialogue may name items and quests, and quests may name items. A spec may refer only to assets from files that sort before its own (or its own).
- **Uniqueness.** Asset names (`DA_...`) and ids (`item_id`, `quest_id`, `dialogue_id`) must be unique across all files of a kind. A clash stops the script and names both files.
- **Ids are forever.** Item ids, quest ids, stage ids, flags, and location ids are stored in saves. Never rename a shipped one. Phase 6 ids will be minted in `Design/POIs/sombre_ids.md` (created in VS-07; the Integrator owns it); until it exists, use only ids the narrative documents and the plan already name.
- **Ownership.** `items/shore.py`, `quests/shore_watch.py`, and `dialogue/mara_intro.py` are accepted Shore content: change them only in an Integrator task. Phase 6 adds files named for the area, for example `items/sombre.py`, `quests/sombre_characteristic.py`, `dialogue/sombre_varga.py`.

## Changing the generators

Anything that changes how a spec becomes an asset (`create_*.py`, `content_specs.py`, the shapes above) is an Integrator change and must be **behavior-preserving for the accepted content** unless its task says otherwise:

```
Tools\DumpContent.bat before
(change, then)  Tools\RebuildContent.bat create_items   (and create_quest, create_dialogue)
Tools\DumpContent.bat after
git diff --no-index Saved\ContentDumps\before.json Saved\ContentDumps\after.json     # must be empty
```

Then `Tools\RunTests.bat` (`DeadCurrent.Content.Validate` checks every quest and dialogue asset's references).

## Adding content

1. Add your file to the matching folder with the list defined.
2. `Tools\RebuildContent.bat create_items` (or `create_quest`, `create_dialogue`).
3. `Tools\RunTests.bat Content` (validates the new assets), and your area's tests.
4. Commit the spec file and the generated assets it produced. Do not commit other regenerated assets.
