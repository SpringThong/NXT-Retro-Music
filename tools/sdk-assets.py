#!/usr/bin/env python3
"""Download pinned SDK assets and extract into a user-owned, relocatable tree."""
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import shutil
import sys
import tarfile
import time
import urllib.request

ROOT = Path(__file__).resolve().parents[1]


def extract(archive, destination, strip=0):
    destination = Path(destination).resolve()
    destination.mkdir(parents=True, exist_ok=True)
    links = []
    with tarfile.open(archive, "r:*") as source:
        for member in source:
            parts = PurePosixPath(member.name).parts
            if ".." in parts or (parts and parts[0] == "/"):
                raise ValueError("Unsafe archive path: " + member.name)
            parts = parts[strip:]
            if not parts:
                continue
            target = destination.joinpath(*parts)
            if not target.resolve().is_relative_to(destination):
                raise ValueError("Archive entry escapes destination")
            target.parent.mkdir(parents=True, exist_ok=True)
            if member.isdir():
                target.mkdir(exist_ok=True)
            elif member.isfile():
                with source.extractfile(member) as incoming, target.open("wb") as outgoing:
                    shutil.copyfileobj(incoming, outgoing, 1024 * 1024)
                target.chmod(0o755 if member.mode & 0o111 else 0o644)
            elif member.issym() or member.islnk():
                links.append((target, member.linkname, member.islnk()))
            else:
                raise ValueError("Unexpected special file in SDK archive")
    for target, linkname, hard in links:
        if hard:
            link = destination.joinpath(*PurePosixPath(linkname).parts[strip:])
        elif linkname.startswith("/"):
            link = destination / linkname.lstrip("/")
        else:
            link = target.parent / linkname
        if not link.resolve().is_relative_to(destination):
            raise ValueError("SDK link escapes destination: " + str(target))
        target.unlink(missing_ok=True)
        if hard:
            os.link(link, target)
        else:
            target.symlink_to(os.path.relpath(link, target.parent))


def download(url, checksum, cache):
    filename = url.rsplit("/", 1)[-1]
    target = cache / filename
    if target.is_file():
        with target.open("rb") as existing:
            if hashlib.file_digest(existing, "sha256").hexdigest() == checksum:
                print("Dùng lại file đã xác minh:", filename, flush=True)
                return target
    temporary = target.with_suffix(target.suffix + ".part")
    for attempt in range(3):
        try:
            request = urllib.request.Request(url, headers={"User-Agent": "NXT-Retro-Music-WSL"})
            with urllib.request.urlopen(request, timeout=45) as response, temporary.open("wb") as output:
                total = int(response.headers.get("Content-Length", "0"))
                count, reported = 0, -1
                digest = hashlib.sha256()
                while chunk := response.read(1024 * 1024):
                    output.write(chunk)
                    digest.update(chunk)
                    count += len(chunk)
                    progress = count * 10 // total if total else count // (25 * 1024 * 1024)
                    if progress != reported:
                        print(f"Tải {filename}: {count // 1024 // 1024} MiB", flush=True)
                        reported = progress
                if digest.hexdigest() != checksum:
                    raise ValueError("SHA256 không khớp: " + filename)
            temporary.replace(target)
            return target
        except Exception:
            temporary.unlink(missing_ok=True)
            if attempt == 2:
                raise
            print("Tải chưa thành công, thử lại...", flush=True)
            time.sleep(2)


def main():
    if len(sys.argv) > 1 and sys.argv[1] == "extract":
        extract(sys.argv[2], sys.argv[3], int(sys.argv[4]) if len(sys.argv) > 4 else 0)
        return
    config = json.loads((ROOT / "SOURCE.json").read_text())
    sdk = ROOT / ".nxt-sdk/tg5040"
    marker = sdk / ".assets-ready"
    if marker.is_file():
        if marker.read_text().strip() != config["compiler_sha256"] + ":" + config["sysroot_sha256"]:
            raise RuntimeError("SDK đã có thuộc phiên bản khác. Không ghi đè; dùng thư mục dự án mới.")
        print("SDK đã có đúng phiên bản.")
        return
    cache = Path(os.environ.get("NXT_DOWNLOAD_CACHE", str(ROOT / ".cache/downloads")))
    cache.mkdir(parents=True, exist_ok=True)
    compiler = download(config["compiler_url"], config["compiler_sha256"], cache)
    sysroot = download(config["sysroot_url"], config["sysroot_sha256"], cache)
    staging = sdk.parent / (".tg5040-staging-" + str(os.getpid()))
    if sdk.exists():
        raise RuntimeError("SDK thiếu marker hoặc chưa hoàn tất. Không ghi đè thư mục hiện có: " + str(sdk))
    try:
        print("Giải nén compiler...", flush=True)
        # PurePosixPath removes the leading './' component of this archive;
        # strip its remaining x86_64-aarch64-nextui-linux-gnu directory only.
        extract(compiler, staging / "compiler", 1)
        print("Giải nén sysroot...", flush=True)
        extract(sysroot, staging / "compiler/aarch64-nextui-linux-gnu/libc")
        # Some vendor libexec ELF tools are incorrectly archived as 0644.
        for folder in [staging/"compiler/bin",staging/"compiler/libexec"]:
            for tool in folder.rglob("*"):
                if tool.is_file() and not tool.is_symlink():
                    with tool.open("rb") as stream:magic=stream.read(4)
                    if magic==b"\x7fELF":tool.chmod(0o755)
        (staging / "local").mkdir()
        (staging / ".assets-ready").write_text(config["compiler_sha256"] + ":" + config["sysroot_sha256"] + "\n")
        staging.rename(sdk)
    finally:
        if staging.exists():
            shutil.rmtree(staging)


if __name__ == "__main__":
    main()
