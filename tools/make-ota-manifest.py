#!/usr/bin/env python3
"""Create the public OTA manifest for an official NXT Stock installer ZIP."""
import argparse
import hashlib
import json
import re
import zipfile
from pathlib import Path
from urllib.parse import urlsplit

PACKAGE = "vn.nguyenxuanthong.nxtretromusic"
ROM = "stock"
DEVICE = "trimui-brick-pro"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", choices=["stock","nextui","nxredux"], default="stock")
    parser.add_argument("--zip", required=True, type=Path, help="Stock installer ZIP")
    parser.add_argument("--url", required=True, help="Public HTTPS download link for that ZIP")
    parser.add_argument("--version", required=True, help="Numeric release version, e.g. 1.5")
    parser.add_argument("--notes", type=Path, help="UTF-8 text file with release notes")
    parser.add_argument("--out", required=True, type=Path, help="Output update.json")
    args = parser.parse_args()
    if not re.fullmatch(r"\d{1,4}(?:\.\d{1,4}){0,3}", args.version):
        parser.error("Version must contain 1–4 numeric components, each from 0 to 9999.")
    url = urlsplit(args.url)
    if url.scheme != "https" or not url.netloc or url.username or url.password or any(c.isspace() for c in args.url):
        parser.error("Download URL must be a public HTTPS link without credentials or spaces.")
    if not args.zip.is_file() or not 0 < args.zip.stat().st_size <= 256 * 1024 * 1024:
        parser.error("Installer ZIP must be a regular file of at most 256 MiB.")
    rom=args.rom
    prefix = {"stock":"Apps/NXT_Retro_Music/","nextui":"Tools/tg5040/NXT Retro Music.pak/","nxredux":"Tools/NXT Retro Music.pak/"}[rom]
    try:
        with zipfile.ZipFile(args.zip) as archive:
            config = json.loads(archive.read(prefix + "config.json"))
            for field, expected in (("package", PACKAGE), ("rom", rom), ("device", DEVICE), ("version", args.version)):
                if config.get(field) != expected:
                    parser.error(f"Installer config field {field!r} does not match {expected!r}.")
            if prefix + "launch.sh" not in archive.namelist():
                parser.error("Installer is missing launch.sh.")
            corrupt = archive.testzip()
            if corrupt:
                parser.error(f"Installer CRC failed: {corrupt}")
    except (OSError, KeyError, ValueError, zipfile.BadZipFile) as error:
        parser.error(f"Cannot validate installer ZIP: {error}")
    digest = hashlib.sha256()
    with args.zip.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    notes = args.notes.read_text(encoding="utf-8") if args.notes else "NXT Retro Music " + args.version
    manifest = {
        "schema": 1, "package": PACKAGE, "rom": rom, "device": DEVICE,
        "version": args.version, "download_url": args.url,
        "sha256": digest.hexdigest(), "size": args.zip.stat().st_size,
        "changelog": notes,
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"Created {args.out} ({manifest['size']} bytes, SHA-256 {manifest['sha256']})")


if __name__ == "__main__":
    main()
