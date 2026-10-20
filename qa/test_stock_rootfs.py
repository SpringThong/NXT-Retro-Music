#!/usr/bin/env python3
"""Run shipped ARM64 payloads against an extracted official Brick Pro rootfs."""
from pathlib import Path
import argparse
import json
import os
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument("--rootfs",type=Path,default=ROOT/".cache/stock-research/rootfs")
parser.add_argument("--qemu",type=Path,default=ROOT/".host-tools/usr/bin/qemu-aarch64")
args=parser.parse_args()
rom=args.rootfs.resolve()
app=ROOT/"dist/stock-stage/Apps/NXT_Retro_Music"
env=os.environ.copy()
hostlibs=ROOT/".host-tools/usr/lib/x86_64-linux-gnu"
if hostlibs.is_dir(): env["LD_LIBRARY_PATH"]=str(hostlibs)
base=[str(args.qemu),"-L",str(rom),"-E",
      "LD_LIBRARY_PATH="+str(app/"bin/tg5040")+":/usr/trimui/lib:/usr/lib:/lib"]
checks={}
def run(name,arguments=(),**kwargs):
    command=base[:]
    label=kwargs.pop("label",name)
    library_dir=kwargs.pop("library_dir",None)
    if library_dir: command+=["-E","LD_LIBRARY_PATH="+str(library_dir)+":/usr/trimui/lib:/usr/lib:/lib"]
    p=subprocess.run(command+[str(app/"bin/tg5040"/name),*arguments],
         cwd=app,env=kwargs.pop("env",env),text=True,capture_output=True,timeout=30,**kwargs)
    print(name,arguments,"exit",p.returncode,p.stdout.strip(),p.stderr.strip()[:400],flush=True)
    assert p.returncode==0
    checks[label+" "+" ".join(arguments)]={"exit":p.returncode,"stdout":p.stdout,"stderr":p.stderr}
    return p
mount=Path("/mnt/SDCARD/Apps/NXT_Retro_Music")
mount.parent.mkdir(parents=True,exist_ok=True)
assert not mount.exists() and not mount.is_symlink(),"QA requires an unused simulated SD path"
mount.symlink_to(app,target_is_directory=True)
try:
    help_result=run("musicplayer.elf",["--help"])
    assert "for Stock" in help_result.stdout and "--test-control" not in help_result.stdout
    build=run("musicplayer.elf",["--build-info"])
    assert "Stock · Automatic OTA & UI Performance" in build.stdout
    info=run("musicplayer.elf",["--stock-info"])
    assert "SDL 2.30.8" in info.stdout and "opengles2" in info.stdout and "MISSING" not in info.stdout
    sp=run("nxt-spotify-backend",input='{"cmd":"quit"}\n')
    assert '"event":"ready"' in sp.stdout and '"event":"closed"' in sp.stdout
    with tempfile.TemporaryDirectory(prefix="nxt-stock-auth-") as directory:
        auth_env=env.copy();auth_env.update(NXT_SPOTIFY_CREDENTIALS=str(Path(directory)/"missing-pc.json"),NXT_SPOTIFY_PLAYBACK_CREDS=str(Path(directory)/"missing-qr.json"))
        first=run("nxt-spotify-backend",label="nxt-spotify-backend login without credentials",env=auth_env,input='{"cmd":"login"}\n{"cmd":"quit"}\n')
        assert '"reason":"pair_required"' in first.stdout and '"operation":"login"' in first.stdout and '"event":"closed"' in first.stdout
    with tempfile.TemporaryDirectory(prefix="nxt-stock-qemu-") as directory:
        receiver_fd,sender_fd=os.pipe()
        backend_env=env.copy()
        backend_env.update(NXT_SOUNDCLOUD_PCM_FD="3",
             NXT_SOUNDCLOUD_CREDENTIALS=str(Path(directory)/"soundcloud.json"))
        def pcm_fd():
            if sender_fd!=3: os.dup2(sender_fd,3)
        result=run("nxt-soundcloud-backend",env=backend_env,
             input='{"cmd":"quit"}\n',pass_fds=tuple({3,sender_fd}),preexec_fn=pcm_fd)
        os.close(receiver_fd); os.close(sender_fd)
        assert '"credentials_required":false' in result.stdout and '"event":"error"' not in result.stdout
    # QEMU user mode cannot automatically re-exec an ARM64 PyInstaller
    # bootloader on an x86 host. Start its onedir child code path for this QA
    # only; the installed launcher uses the unchanged native bootloader.
    executable=app/"bin/tg5040/yt-dlp-runtime/yt-dlp"
    python_env=env.copy()
    python_env.update(_PYI_ARCHIVE_FILE=str(executable),
         _PYI_APPLICATION_HOME_DIR=str(executable.parent/"_internal"),
         _PYI_PARENT_PROCESS_LEVEL="1")
    ytdlp=run("yt-dlp-runtime/yt-dlp",["--version"],env=python_env,
              library_dir=executable.parent/"_internal")
    assert ytdlp.stdout.strip()=="2026.08.19"
    (ROOT/"logs/stock-rootfs-test.json").write_text(json.dumps(checks,ensure_ascii=False,indent=2)+"\n")
    print("PASS official stock rootfs: frontend help/resources/SDL drivers, Spotify & SoundCloud startup/quit, bundled yt-dlp.",flush=True)
finally:
    mount.unlink()
