#!/usr/bin/env python3
"""Freeze the complete v1.2 NextUI project and its matching installer."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import shutil
import struct
import zipfile

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--output-dir", type=Path, default=ROOT / "dist/release")
args = parser.parse_args()
output = args.output_dir.resolve()
output.mkdir(parents=True, exist_ok=True)
installer = ROOT / "dist/NXT_Retro_Music_v1.2_NextUI.zip"
guide = ROOT / "HUONG_DAN_NXT_Retro_Music_v1.2_NextUI.md"
prefix = "NXT Retro Music v1.2.pak/"
for filename in ["musicplayer.elf", "nxt-spotify-backend", "nxt-soundcloud-backend"]:
    data = (ROOT / "upstream/bin/tg5040" / filename).read_bytes()
    assert data[:6] == b"\x7fELF\x02\x01" and struct.unpack_from("<H", data, 18)[0] == 183
with zipfile.ZipFile(installer) as archive:
    assert archive.testzip() is None
    metadata = json.loads(archive.read(prefix + "pak.json"))
    assert metadata["name"] == "NXT Retro Music v1.2" and metadata["version"] == "v1.2"
    assert archive.read(prefix + "state/app_version.txt").strip() == b"1.2"
    for filename in ["musicplayer.elf", "nxt-spotify-backend", "nxt-soundcloud-backend"]:
        assert archive.read(prefix + "bin/tg5040/" + filename) == (ROOT / "upstream/bin/tg5040" / filename).read_bytes()
        assert archive.getinfo(prefix + "bin/tg5040/" + filename).external_attr >> 16 & 0o111
    assert archive.getinfo(prefix + "launch.sh").external_attr >> 16 & 0o111
    assert "Spotify_Login_PC/Spotify_Login.py" in archive.namelist()
    assert prefix + "res/soundcloud/nxt/yt_dlp_plugins/extractor/nxt_soundcloud.py" in archive.namelist()
    assert archive.getinfo(prefix + "bin/tg5040/yt-dlp").external_attr >> 16 & 0o111
    assert archive.read(guide.name) == guide.read_bytes()
    assert not any(name.endswith(("credentials.json", "playback.json")) for name in archive.namelist())

for item in json.loads((ROOT / "overlay-manifest.json").read_text())["files"]:
    for base in ["upstream", "overlay"]:
        assert hashlib.sha256((ROOT / base / item["path"]).read_bytes()).hexdigest() == item["sha256"]
assert "librespot-connect" not in (ROOT / "spotify/backend/Cargo.lock").read_text()

skip = {".git", ".cache", ".nxt-sdk", ".host-tools", ".build-headers", "dist", "backups", "__pycache__", "target", "toolchain", "host-sdk"}
files = []
for path in sorted(ROOT.rglob("*")):
    relative = path.relative_to(ROOT)
    if not path.is_file() or path.is_symlink() or set(relative.parts) & skip:
        continue
    if output == path or output in path.parents:
        continue
    if path.name in ["FILES_SHA256.json", "spotify-credentials.json", "credentials.json", "playback.json", "NXT_Spotify_Log.txt", "NXT_Input_Log.txt", "soundcloud-credentials.json", "bookmarks.json", "NXT_SoundCloud_Log.txt"]:
        continue
    if relative.parts[:3] == ("upstream", "bin", "desktop"):
        continue
    if relative.parts[0] == "upstream" and ("build" in relative.parts or path.suffix in [".o", ".d"]):
        continue
    if path.name.endswith(".arm64"):
        continue
    files.append(path)
hashes = {path.relative_to(ROOT).as_posix(): hashlib.sha256(path.read_bytes()).hexdigest() for path in files}
(ROOT / "FILES_SHA256.json").write_text(json.dumps(hashes, indent=2) + "\n")
files.append(ROOT / "FILES_SHA256.json")
source = output / "NXT_Retro_Music_v1.2_NextUI_Source.zip"
temporary_source = source.with_suffix(".zip.building")
prefix = "NXT_Retro_Music_v1.2_NextUI_Source/"
with zipfile.ZipFile(temporary_source, "w", zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
    for path in files:
        archive.write(path, prefix + path.relative_to(ROOT).as_posix())
with zipfile.ZipFile(temporary_source) as archive:
    assert archive.testzip() is None
    for filename, digest in hashes.items():
        assert hashlib.sha256(archive.read(prefix + filename)).hexdigest() == digest, filename
with temporary_source.open("rb") as saved:
    os.fsync(saved.fileno())
os.replace(temporary_source, source)
for original in [installer, guide]:
    destination = output / original.name
    temporary = destination.with_suffix(destination.suffix + ".building")
    with original.open("rb") as reader, temporary.open("wb") as writer:
        shutil.copyfileobj(reader, writer)
        writer.flush()
        os.fsync(writer.fileno())
    os.replace(temporary, destination)
with zipfile.ZipFile(output / installer.name) as archive:
    assert archive.testzip() is None
for path in [output / installer.name, source]:
    print(path.name, path.stat().st_size, hashlib.sha256(path.read_bytes()).hexdigest(), flush=True)
print("PASS: v1.2 metadata, AArch64 payload, executable modes, ZIP CRC, source hashes, matching overlay and guide; no account tokens or Connect service.", flush=True)
