"""Generate one Meshy prop: preview, thumbnail check, refine, download, source.json.

The API key is read from MESHY_API_KEY or C:\\FO5_AssetLibrary\\Meshy\\meshy_key.txt.
It is never printed, logged, or written into source.json.

  py -3 Tools/generate_meshy.py relay_housing
      Starts a preview if none is waiting, downloads the thumbnail, and stops.
  py -3 Tools/generate_meshy.py relay_housing --refine --note "what the thumbnail showed"
      Textures the checked preview, downloads the FBX and PBR maps, and records source.json.
  py -3 Tools/generate_meshy.py relay_housing --reject "why the preview is wrong"
      Marks that preview failed and starts another. Does not refine it.
"""

import argparse
import json
import os
import sys
import time
import urllib.error
import urllib.request

API = "https://api.meshy.ai/openapi/v2/text-to-3d"
BALANCE = "https://api.meshy.ai/openapi/v1/balance"
KEY_PATH = r"C:\FO5_AssetLibrary\Meshy\meshy_key.txt"
OUT_ROOT = r"C:\FO5_AssetLibrary\Meshy"
POLL_SECONDS = 8
TIMEOUT_SECONDS = 20 * 60

# The Presentation Pass Meshy queue (PresentationPassPlan.txt section 6). Budget ceiling: 500 credits.
# A preview costs about 20 and a refine about 10. Never refine a preview the thumbnail check rejected.
PROPS = {
    "depth_sounder": {
        "prompt": (
            "A 1970s boat depth sounder, a boxy painted steel housing about 28 centimeters wide, "
            "18 deep, and 16 tall. Flat front panel with one round recessed dial, one large "
            "knob, and a narrow slot on top where a paper roll would feed. Chunky, solid, "
            "sitting flat on its base. No brand, no logo, no letters, no screen, no loose wires."
        ),
        "texture_prompt": (
            "Faded grey-green marine enamel chipped to rust at the corners, freshwater stains, "
            "one smeared white grease-pencil circle on the front panel, no logo, no lettering"
        ),
        "target_polycount": 10000,
    },
    "breaker_panel": {
        "prompt": (
            "A marine breaker panel, about 40 centimeters wide, 55 tall, and 8 deep. A flat steel "
            "back plate with a hinged front cover swung open, two rows of chunky breaker "
            "switches, and thick cut cables hanging from the bottom with taped ends. Solid, "
            "chunky, oxidized steel. No logo, no brand, no letters, no screen."
        ),
        "texture_prompt": (
            "Oxidized steel with faded pale grey maritime paint, freshwater rust stains, dull "
            "black tape on the cable ends, no logo, no lettering"
        ),
        "target_polycount": 15000,
    },
    "battery_bank": {
        "prompt": (
            "A split wooden box of old marine batteries, about 50 centimeters wide, 35 deep, and "
            "30 tall. Weathered planks, one side split open, showing three heavy battery cells "
            "with corroded terminals and two thick heavy cables with clamps trailing out. "
            "Solid and chunky, sitting flat on the ground. No logo, no brand, no letters."
        ),
        "texture_prompt": (
            "Grey weathered wood, black battery cases with white and green corrosion crust on "
            "the terminals, dull black cables, freshwater stains, no logo, no lettering"
        ),
        "target_polycount": 15000,
    },
    "emergency_beacon": {
        "prompt": (
            "A small boat emergency beacon, about 12 centimeters wide, 12 deep, and 22 tall. "
            "Rounded oval body, a short stubby antenna, one large recessed on-off switch, and a "
            "small bracket base. Solid and chunky, standing upright. No brand, no logo, no "
            "letters, no screen."
        ),
        "texture_prompt": (
            "White plastic gone yellow-grey, a crust of white battery corrosion around the "
            "base, scuffed, freshwater stains, no logo, no lettering"
        ),
        "target_polycount": 8000,
    },
    "name_board_tern": {
        "prompt": (
            "A flat weathered wooden name board from a small boat, about 70 centimeters long, "
            "20 tall, and 2 thick. A plain slab with slightly bevelled edges and two screw "
            "holes near the ends, worn corners. Bare board, no letters, no text, no logo."
        ),
        "texture_prompt": (
            "Peeling white and teal marine paint over grey weathered wood, flaked and stained, "
            "bare wood showing at the edges, no letters, no text, no logo"
        ),
        "target_polycount": 5000,
    },
    "sounder_chart": {
        "prompt": (
            "A tightly closed roll of paper chart, about 6 centimeters across and 24 long, "
            "lying on its side. A solid closed cylinder of wound paper with a thin band of tape "
            "around the middle. No open sheet, no unrolled tail, no text."
        ),
        "texture_prompt": (
            "Aged cream-yellow paper with water stains, faint pencil tick marks along the outer "
            "edge, grey tape band, no letters, no text"
        ),
        "target_polycount": 4000,
    },
    "radio_coil": {
        "prompt": (
            "A hand-wound copper wire coil, about 10 centimeters across and 8 tall. Tightly "
            "wound wire on a bare core, no case, no housing, two short stub wire ends. Solid "
            "and chunky. No brand, no logo, no letters."
        ),
        "texture_prompt": (
            "Dull oxidized copper with green verdigris, a wrap of black electrical tape, grime, "
            "no lettering"
        ),
        "target_polycount": 8000,
    },
    "dead_fish": {
        "prompt": (
            "One small dead freshwater fish lying belly up, about 28 centimeters long, 8 tall, "
            "and 6 wide. One burst clouded eye, mouth slightly open, fins limp. A perch-like "
            "body. Solid single mesh, no water, no base."
        ),
        "texture_prompt": (
            "Dull grey-green scales with dark bars, pale belly, cloudy white eye, wet dull "
            "sheen, no letters"
        ),
        "target_polycount": 8000,
    },
    "field_dressing": {
        "prompt": (
            "A folded roll of grubby cloth bandage, about 10 centimeters long, 6 wide, and 4 "
            "tall, with a short strip of tape wrapped around the middle. A compact thick roll "
            "lying flat. No cross symbol, no logo, no letters."
        ),
        "texture_prompt": (
            "Off-white cotton gone grey-yellow with stains, a cream tape strip, no cross, no "
            "logo, no lettering"
        ),
        "target_polycount": 5000,
    },
    "relay_housing": {
        "prompt": (
            "A single open steel relay box, about 40 centimeters wide, 28 deep, and 22 tall. "
            "One hinged lid only, propped up. The cavity holds three thick metal terminal lugs "
            "with dull tape wrapped around them and short stubby cut cable ends. Solid chunky "
            "parts, not thin wires, not an empty box. Oxidized steel housing. No logo, no brand, "
            "no letters, no screen, no second door."
        ),
        "texture_prompt": (
            "Oxidized steel with faded cool grey maritime paint, freshwater rust stains and "
            "grime, dull tape on the coil lugs, no logo, no brand, no lettering"
        ),
        "target_polycount": 20000,
    },
    # Presentation Pass playtest 1 (2026-09-29): the cot was a white block, and Mara needed her own face.
    "field_cot": {
        "prompt": (
            "A folding army field cot, about 190 centimeters long, 70 wide, and 45 tall. A "
            "steel tube frame with crossed scissor legs, and a taut canvas sleeping surface "
            "stretched between two long side rails, sagging slightly in the middle. A rolled "
            "grey wool blanket lies at the head end. Chunky, solid, sitting flat on the ground. "
            "No logo, no brand, no letters, no pillow."
        ),
        "texture_prompt": (
            "Faded olive-drab canvas stiff with white salt tide-line stains, dull oxidized "
            "steel frame with rust at the joints, grey wool blanket, no logo, no lettering"
        ),
        "target_polycount": 12000,
    },
    "mara_head": {
        "prompt": (
            "A realistic adult woman's head and neck only, about 22 centimeters tall from the "
            "chin to the crown, in a neutral A-pose bust style. Late thirties, weathered "
            "wind-chapped skin, calm watchful eyes, straight relaxed mouth closed, strong "
            "cheekbones, dark hair cut short and tied back. The neck ends in a flat horizontal "
            "cut. No clothing, no hat, no jewelry, no glasses, no shoulders, no body."
        ),
        "texture_prompt": (
            "Realistic human skin, weathered and slightly sunburnt, small freckles, dark "
            "brown hair, grey-green eyes, matte, no makeup, no tattoos"
        ),
        "target_polycount": 15000,
    },
}


def load_key():
    key = os.environ.get("MESHY_API_KEY", "").strip()
    if not key and os.path.isfile(KEY_PATH):
        with open(KEY_PATH, "r", encoding="utf-8") as handle:
            key = handle.read().strip()
    if not key:
        raise SystemExit(f"No Meshy key in MESHY_API_KEY or {KEY_PATH}")
    return key


def request(key, method, url, body=None):
    data = None if body is None else json.dumps(body).encode("utf-8")
    req = urllib.request.Request(url, data=data, method=method)
    req.add_header("Authorization", "Bearer " + key)
    if body is not None:
        req.add_header("Content-Type", "application/json")
    try:
        with urllib.request.urlopen(req, timeout=60) as response:
            raw = response.read()
    except urllib.error.HTTPError as error:
        detail = error.read().decode("utf-8", errors="replace")
        raise SystemExit(f"Meshy {method} {error.code}: {detail}") from None
    if not raw:
        return {}
    return json.loads(raw.decode("utf-8"))


def balance(key):
    payload = request(key, "GET", BALANCE)
    return int(payload.get("balance", 0))


def create_task(key, body):
    payload = request(key, "POST", API, body)
    task_id = payload.get("result")
    if not task_id:
        raise SystemExit("Meshy did not return a task id")
    return task_id


def poll(key, task_id):
    deadline = time.time() + TIMEOUT_SECONDS
    while time.time() < deadline:
        task = request(key, "GET", f"{API}/{task_id}")
        status = task.get("status", "")
        progress = task.get("progress", 0)
        print(f"{task_id} {status} {progress}", flush=True)
        if status == "SUCCEEDED":
            return task
        if status in ("FAILED", "CANCELED"):
            message = ""
            err = task.get("task_error") or {}
            if isinstance(err, dict):
                message = err.get("message", "")
            raise SystemExit(f"Task {task_id} {status}: {message}")
        time.sleep(POLL_SECONDS)
    raise SystemExit(f"Task {task_id} timed out")


def download(url, dest):
    urllib.request.urlretrieve(url, dest)
    print(f"saved {dest}", flush=True)


def prop_dir(prop_id):
    dest = os.path.join(OUT_ROOT, prop_id)
    os.makedirs(dest, exist_ok=True)
    return dest


def source_path(prop_id):
    return os.path.join(prop_dir(prop_id), "source.json")


def load_source(prop_id):
    path = source_path(prop_id)
    if not os.path.isfile(path):
        return None
    with open(path, "r", encoding="utf-8") as handle:
        return json.load(handle)


def save_source(prop_id, record):
    path = source_path(prop_id)
    with open(path, "w", encoding="utf-8") as handle:
        json.dump(record, handle, indent=2)
        handle.write("\n")
    print(f"wrote {path}", flush=True)


def blank_record(spec):
    return {
        "id": spec_id_from(spec),
        "date": time.strftime("%Y-%m-%d"),
        "ai_model": "latest",
        "generated_with": "generated with the owner's Meshy Pro account",
        "target_polycount": spec["target_polycount"],
        "preview_prompts": [],
        "texture_prompt": spec["texture_prompt"],
        "task_ids": {},
        "thumbnail_check": "",
        "credits_used": 0,
        "balance_after": None,
        "status": "new",
    }


def spec_id_from(spec):
    return spec["_id"]


def start_preview(key, prop_id, spec, record):
    before = balance(key)
    task_id = create_task(key, {
        "mode": "preview",
        "prompt": spec["prompt"],
        "ai_model": "latest",
        "should_remesh": True,
        "topology": "triangle",
        "target_polycount": spec["target_polycount"],
        "target_formats": ["fbx"],
    })
    record["preview_prompts"].append(spec["prompt"])
    index = sum(1 for key_name in record["task_ids"] if key_name.startswith("preview"))
    record["task_ids"][f"preview_{index + 1}"] = task_id
    record["status"] = "preview_running"
    record["balance_before_preview"] = before
    save_source(prop_id, record)
    task = poll(key, task_id)
    thumb = task.get("thumbnail_url")
    if not thumb:
        raise SystemExit(f"Preview {task_id} has no thumbnail")
    download(thumb, os.path.join(prop_dir(prop_id), "thumbnail.png"))
    record["status"] = "awaiting_check"
    record["credits_used"] = int(record.get("credits_used") or 0) + int(task.get("consumed_credits") or 0)
    record["balance_after"] = balance(key)
    save_source(prop_id, record)
    print(f"CHECK {os.path.join(prop_dir(prop_id), 'thumbnail.png')}", flush=True)
    print(f"credits_used={record['credits_used']} balance_after={record['balance_after']}", flush=True)


def refine(key, prop_id, spec, record, note):
    if record.get("status") != "awaiting_check":
        raise SystemExit(f"{prop_id} is {record.get('status')}, not awaiting a thumbnail check")
    preview_id = None
    for name, task_id in record["task_ids"].items():
        if name.startswith("preview") and "failed" not in name:
            preview_id = task_id
    if not preview_id:
        raise SystemExit("No preview task to refine")
    record["thumbnail_check"] = note
    task_id = create_task(key, {
        "mode": "refine",
        "preview_task_id": preview_id,
        "enable_pbr": True,
        "texture_prompt": spec["texture_prompt"],
        "ai_model": "latest",
        "target_formats": ["fbx"],
    })
    record["task_ids"]["refine"] = task_id
    record["task_ids"]["refined_from_preview"] = preview_id
    record["status"] = "refine_running"
    save_source(prop_id, record)
    task = poll(key, task_id)
    folder = prop_dir(prop_id)
    model_urls = task.get("model_urls") or {}
    if not model_urls.get("fbx"):
        raise SystemExit("Refine finished without an FBX url")
    download(model_urls["fbx"], os.path.join(folder, "model.fbx"))
    if model_urls.get("glb"):
        download(model_urls["glb"], os.path.join(folder, "model.glb"))
    names = {
        "base_color": "base_color.png",
        "metallic": "metallic.png",
        "roughness": "roughness.png",
        "normal": "normal.png",
    }
    for entry in task.get("texture_urls") or []:
        if not isinstance(entry, dict):
            continue
        for kind, filename in names.items():
            if entry.get(kind):
                download(entry[kind], os.path.join(folder, filename))
    if task.get("thumbnail_url"):
        download(task["thumbnail_url"], os.path.join(folder, "thumbnail_refine.png"))
    record["credits_used"] = int(record.get("credits_used") or 0) + int(task.get("consumed_credits") or 0)
    record["balance_after"] = balance(key)
    record["status"] = "ready"
    save_source(prop_id, record)
    print(f"credits_used={record['credits_used']} balance_after={record['balance_after']}", flush=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("prop_id")
    parser.add_argument("--refine", action="store_true")
    parser.add_argument("--reject", default="")
    parser.add_argument("--note", default="")
    args = parser.parse_args()
    if args.prop_id not in PROPS:
        raise SystemExit(f"Unknown prop {args.prop_id}")
    spec = dict(PROPS[args.prop_id])
    spec["_id"] = args.prop_id
    if len(spec["prompt"]) > 800 or len(spec["texture_prompt"]) > 800:
        raise SystemExit("Prompt exceeds 800 characters")
    key = load_key()
    record = load_source(args.prop_id) or blank_record(spec)
    record["id"] = args.prop_id
    if args.reject:
        if record.get("status") != "awaiting_check":
            raise SystemExit("Nothing is awaiting a check")
        failed = [name for name in record["task_ids"] if name.startswith("preview") and "failed" not in name]
        if not failed:
            raise SystemExit("No preview to reject")
        name = failed[-1]
        record["task_ids"][name + "_failed"] = record["task_ids"].pop(name)
        prompts = record["preview_prompts"]
        if prompts and not prompts[-1].endswith(")"):
            prompts[-1] = prompts[-1] + f" (FAILED: {args.reject})"
        record["status"] = "rejected"
        record["thumbnail_check"] = args.reject
        save_source(args.prop_id, record)
        start_preview(key, args.prop_id, spec, record)
        return
    if args.refine:
        if not args.note:
            raise SystemExit("Pass --note with what the thumbnail showed")
        refine(key, args.prop_id, spec, record, args.note)
        return
    if record.get("status") == "awaiting_check":
        print(f"CHECK {os.path.join(prop_dir(args.prop_id), 'thumbnail.png')}", flush=True)
        return
    if record.get("status") == "ready":
        print(f"{args.prop_id} is already ready", flush=True)
        return
    if record.get("status") == "preview_running":
        running = [name for name in record["task_ids"] if name.startswith("preview") and "failed" not in name]
        if not running:
            raise SystemExit("Preview was marked running but no task id was saved")
        task = poll(key, record["task_ids"][running[-1]])
        thumb = task.get("thumbnail_url")
        if not thumb:
            raise SystemExit("Preview finished without a thumbnail")
        download(thumb, os.path.join(prop_dir(args.prop_id), "thumbnail.png"))
        record["status"] = "awaiting_check"
        record["credits_used"] = int(record.get("credits_used") or 0) + int(task.get("consumed_credits") or 0)
        record["balance_after"] = balance(key)
        save_source(args.prop_id, record)
        print(f"CHECK {os.path.join(prop_dir(args.prop_id), 'thumbnail.png')}", flush=True)
        print(f"credits_used={record['credits_used']} balance_after={record['balance_after']}", flush=True)
        return
    start_preview(key, args.prop_id, spec, record)


if __name__ == "__main__":
    main()
