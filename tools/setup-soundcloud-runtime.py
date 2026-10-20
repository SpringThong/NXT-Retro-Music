#!/usr/bin/env python3
"""Restore the pinned official yt-dlp ARM64 onedir runtime; includes Python."""
from pathlib import Path
import hashlib, json, shutil, urllib.request, zipfile

r = Path(__file__).resolve().parents[1]
pin = json.loads((r/'soundcloud/runtime.json').read_text())
destination = r/'upstream/bin/tg5040/yt-dlp-runtime'
manifest = json.loads((r/'soundcloud/runtime-files.json').read_text())
if all((destination/name).is_file() and hashlib.sha256((destination/name).read_bytes()).hexdigest()==digest
       for name, digest in manifest.items()):
    print('Bundled yt-dlp runtime verified:', pin['version'])
    raise SystemExit(0)
cache = r/'.cache/downloads'
cache.mkdir(parents=True, exist_ok=True)
archive = cache/'yt-dlp_linux_aarch64.zip'
if not archive.is_file() or hashlib.sha256(archive.read_bytes()).hexdigest() != pin['sha256']:
    temporary = archive.with_suffix('.zip.part')
    with urllib.request.urlopen(pin['url'], timeout=60) as source, temporary.open('wb') as target:
        shutil.copyfileobj(source, target)
    if hashlib.sha256(temporary.read_bytes()).hexdigest() != pin['sha256']:
        temporary.unlink()
        raise SystemExit('SHA256 của runtime yt-dlp không khớp.')
    temporary.replace(archive)
destination = r/'upstream/bin/tg5040/yt-dlp-runtime'
staging = destination.with_name('yt-dlp-runtime.staging')
shutil.rmtree(staging, ignore_errors=True)
staging.mkdir()
with zipfile.ZipFile(archive) as z:
    for item in z.infolist():
        path = Path(item.filename)
        if path.is_absolute() or '..' in path.parts:
            raise SystemExit('Đường dẫn runtime không hợp lệ.')
        z.extract(item, staging)
        if not item.is_dir():
            (staging/path).chmod((item.external_attr>>16)&0o777 or 0o644)
(staging/'yt-dlp_linux_aarch64').rename(staging/'yt-dlp')
assert (staging/'yt-dlp').read_bytes()[:6] == b'\x7fELF\x02\x01'
shutil.rmtree(destination, ignore_errors=True)
staging.rename(destination)
launcher = destination.parent/'yt-dlp'
launcher.write_text('#!/bin/sh\nNXT_YTDLP_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"\nexec "$NXT_YTDLP_DIR/yt-dlp-runtime/yt-dlp" "$@"\n')
launcher.chmod(0o755)
print('Verified and restored yt-dlp', pin['version'], 'AArch64 + bundled Python.')
