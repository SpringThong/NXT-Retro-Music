#!/usr/bin/env python3
"""Package a standalone Apps payload for official TrimUI Brick Pro firmware."""
from ota_source_guard import publisher_source
from pathlib import Path
import hashlib
import json
import os
import shutil
import struct
import zipfile
ROOT=Path(__file__).resolve().parents[1]
STAGE=ROOT/"dist/stock-stage"
APP=STAGE/"Apps/NXT_Retro_Music"
OUT=ROOT/"dist/stock-release"
def copy(source,target):
    target.parent.mkdir(parents=True,exist_ok=True)
    if source.is_dir():
        shutil.copytree(source,target,dirs_exist_ok=True,ignore=shutil.ignore_patterns("__pycache__", ".*"))
    else: shutil.copy2(source,target)
def main():
    if STAGE.exists(): shutil.rmtree(STAGE)
    APP.mkdir(parents=True); OUT.mkdir(parents=True,exist_ok=True)
    ota_url=publisher_source(ROOT/'stock/ota-source-stock.url')
    for relative in ["config.json","launch.sh"]: copy(ROOT/"stock"/relative,APP/relative)
    copy(ROOT/"upstream/res/nxt-logo.png",APP/"icon.png")
    for relative in ["res","licenses","stations","state/app_version.txt"]:
        copy(ROOT/"upstream"/relative,APP/relative)
    for path in (ROOT/"upstream/bin/tg5040").iterdir():
        if path.name.startswith(".") or path.name in ["musicplayer.elf","libmsettings.so"]: continue
        copy(path,APP/"bin/tg5040"/path.name)
    for name in ["musicplayer.elf","libmsettings.so"]:
        copy(ROOT/"stock/bin/tg5040"/name,APP/"bin/tg5040"/name)
    copy(ROOT/"upstream/NextUI/skeleton/SYSTEM/res",APP/"compat/res")
    copy(ROOT/"upstream/NextUI/LICENSE",APP/"licenses/NextUI_GPL-3.0.txt")
    copy(ROOT/"upstream/LICENSE",APP/"LICENSE")
    copy(ROOT/'docs/THEM_MOI_v1.9.1.md',STAGE/'THEM_MOI_v1.9.1.md')
    version=json.loads((ROOT/"stock/config.json").read_text())["version"]
    for note in [f"LIBRARY_v{version}.md",f"DSD_v{version}_Stock.md"]: copy(ROOT/"docs"/note,STAGE/note)
    guide=ROOT/("docs/HUONG_DAN_NXT_Retro_Music_v"+version+"_Stock.md")
    copy(guide,STAGE/guide.name)
    copy(ROOT/("CHANGELOG_v"+version+"_Stock.md"),STAGE/("CHANGELOG_v"+version+"_Stock.md"))
    copy(ROOT/"Spotify_Login_PC",STAGE/"Spotify_Login_PC")
    copy(ROOT/("docs/OTA_Google_Drive_v"+version),STAGE/("OTA_Google_Drive_v"+version))
    if (ROOT/"stock/ota-source-stock.url").is_file():copy(ROOT/"stock/ota-source-stock.url",APP/"ota-source-stock.url")
    (APP/'ota-source-stock.url').write_text(ota_url+'\n')
    for name in ["musicplayer.elf","nxt-spotify-backend","nxt-soundcloud-backend"]:
        data=(APP/"bin/tg5040"/name).read_bytes()
        assert data[:6]==b"\x7fELF\x02\x01" and struct.unpack_from("<H",data,18)[0]==183,name
        (APP/"bin/tg5040"/name).chmod(0o755)
    (APP/"launch.sh").chmod(0o755)
    (APP/"bin/tg5040/yt-dlp").chmod(0o755)
    (APP/"bin/tg5040/yt-dlp-runtime/yt-dlp").chmod(0o755)
    assert json.loads((APP/"config.json").read_text())["launch"]=="launch.sh"
    for filename in ["font1.ttf","font2.ttf","assets@3x.png","inputs@3x.png"]:
        assert (APP/"compat/res"/filename).is_file()
    filename=json.loads((ROOT/"stock/config.json").read_text())["release_filename"]
    assert Path(filename).name==filename and filename.endswith(".zip")
    archive=OUT/filename
    temporary=archive.with_suffix(".zip.building")
    forbidden={"groq-api-key.txt","groq-key.txt","translation-service.json", "credentials.json","spotify-credentials.json","soundcloud-credentials.json",
               "playback.json","NXT_Input_Log.txt","NXT_Spotify_Log.txt","NXT_SoundCloud_Log.txt"}
    with zipfile.ZipFile(temporary,"w",zipfile.ZIP_DEFLATED,compresslevel=6) as z:
        for path in sorted(STAGE.rglob("*")):
            if path.is_file() and path.name not in forbidden:
                relative=path.relative_to(STAGE).as_posix()
                info=zipfile.ZipInfo.from_file(path,relative);info.compress_type=zipfile.ZIP_DEFLATED
                if relative=="Apps/NXT_Retro_Music/launch.sh" or relative.startswith("Apps/NXT_Retro_Music/bin/") and path.name in {"musicplayer.elf","nxt-spotify-backend","nxt-soundcloud-backend","yt-dlp"}:
                    info.create_system=3;info.external_attr=(0o100755<<16)
                with path.open("rb") as source,z.open(info,"w") as target:shutil.copyfileobj(source,target)

    with zipfile.ZipFile(temporary) as z:
        assert z.testzip() is None
        for executable in ["launch.sh","bin/tg5040/musicplayer.elf","bin/tg5040/nxt-spotify-backend","bin/tg5040/nxt-soundcloud-backend","bin/tg5040/yt-dlp","bin/tg5040/yt-dlp-runtime/yt-dlp"]:
            assert z.getinfo("Apps/NXT_Retro_Music/"+executable).external_attr>>16 & 0o111,executable
        assert all(not name.startswith((".system/",".userdata/","Emus/","BIOS/")) for name in z.namelist())
    with temporary.open("rb") as f: os.fsync(f.fileno())
    os.replace(temporary,archive)
    digest=hashlib.sha256(archive.read_bytes()).hexdigest()
    archive.with_suffix(".zip.sha256").write_text(digest+"  "+archive.name+"\n")
    canonical=OUT/("NXT_Retro_Music_v"+version+"_Stock.zip")
    if canonical!=archive:
        shutil.copy2(archive,canonical)
        canonical.with_suffix(".zip.sha256").write_text(digest+"  "+canonical.name+"\n")
    copy(guide,OUT/guide.name)
    print("PASS stock Apps payload, AArch64 binaries, resources, executable modes and ZIP CRC.",flush=True)
    print(archive,archive.stat().st_size,digest,flush=True)
if __name__=="__main__": main()
