#!/usr/bin/env python3
"""Apply the reviewed NXT source overlay and keep replaced local edits."""
import argparse
import datetime
import hashlib
import json
from pathlib import Path
import shutil
import subprocess

args = argparse.ArgumentParser()
args.add_argument("--project", type=Path, default=Path(__file__).resolve().parents[1])
project = args.parse_args().project.resolve()
source = project / "upstream"
manifest = json.loads((project / "overlay-manifest.json").read_text())
if not (source / ".git").exists():
    raise SystemExit("Chưa có upstream. Chạy bash bootstrap-wsl.sh trước.")
origin = subprocess.check_output(["git", "-C", str(source), "remote", "get-url", "origin"], text=True).strip()
if origin != manifest["origin"]:
    raise SystemExit("Origin khác bản nền đã chọn; giữ nguyên mã nguồn hiện có.")
backup = project / "backups" / datetime.datetime.now().strftime("source-%Y%m%d-%H%M%S-%f")
for item in manifest["files"]:
    relative = Path(item["path"])
    if relative.is_absolute() or ".." in relative.parts:
        raise SystemExit("Đường dẫn overlay không hợp lệ.")
    incoming = project / "overlay" / relative
    if hashlib.sha256(incoming.read_bytes()).hexdigest() != item["sha256"]:
        raise SystemExit("Overlay sai checksum: " + str(relative))
    target = source / relative
    if target.is_file() and target.read_bytes() == incoming.read_bytes():
        continue
    if target.is_file() and target.read_bytes() != incoming.read_bytes():
        old_digest = hashlib.sha256(target.read_bytes()).hexdigest()
        if old_digest != item.get("baseline_sha256"):
            saved = backup / relative
            saved.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(target, saved)
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(incoming, target)
    target.touch()
(source / ".nxt-overlay-version").write_text(manifest["version"] + "\n")
print("Đã áp dụng NXT Retro Music " + manifest["version"])
if backup.exists():
    print("Các thay đổi trước đó được giữ tại: " + str(backup))
