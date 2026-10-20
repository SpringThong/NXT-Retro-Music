#!/usr/bin/env python3
"""Update an existing native WSL project while retaining its SDK and local files."""
import argparse
import datetime
from pathlib import Path
import shutil
import subprocess
import sys

parser = argparse.ArgumentParser()
parser.add_argument("project", type=Path)
parser.add_argument("--kit-only", action="store_true")
args = parser.parse_args()
kit = Path(__file__).resolve().parent
target = args.project.expanduser().resolve()
if target == kit:
    if not args.kit_only and (target / "upstream/.git").exists():
        subprocess.run([sys.executable, str(target / "tools/apply-source.py")], check=True)
    raise SystemExit(0)
if target.exists() and any(target.iterdir()):
    if not (target / "SOURCE.json").is_file() or not (target / "tools/build-native.sh").is_file():
        raise SystemExit("Thư mục đích có dữ liệu khác; chọn thư mục dự án NXT đang dùng.")
backup = target / "backups" / datetime.datetime.now().strftime("kit-%Y%m%d-%H%M%S-%f")
excluded = {"upstream", ".nxt-sdk", ".cache", "logs", "dist", "backups", "__pycache__"}
for file in sorted(kit.rglob("*")):
    relative = file.relative_to(kit)
    if any(part in excluded for part in relative.parts) or file.is_symlink() or not file.is_file():
        continue
    out = target / relative
    if out.is_file() and out.read_bytes() != file.read_bytes():
        old = backup / relative
        old.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(out, old)
    out.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(file, out)
if not (target / "upstream").exists() and (kit / "upstream/src/Makefile").is_file():
    shutil.copytree(kit / "upstream", target / "upstream",
                    ignore=shutil.ignore_patterns(".git", "build", "bin", "__pycache__", "*.o", "*.a", "*.so"))
if not args.kit_only and (target / "upstream/.git").exists():
    subprocess.run([sys.executable, str(target / "tools/apply-source.py")], check=True)
print("Đã cập nhật dự án: " + str(target))
print("Tiếp tục: cd \"" + str(target) + "\" && make nextui")
