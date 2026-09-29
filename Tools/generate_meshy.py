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

# Only relay_housing is generated in this pass. The other nine wait on Anthony's review.
PROPS = {
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
