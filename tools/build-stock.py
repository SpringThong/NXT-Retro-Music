#!/usr/bin/env python3
"""Build the selected ROM frontend from source, using the pinned ARM64 library snapshot."""
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import hashlib
import json
import os
import shutil
import subprocess
import tarfile

ROOT = Path(__file__).resolve().parents[1]
ROM = os.environ.get("NXT_BUILD_ROM",json.loads((ROOT / "SOURCE.json").read_text())["build_define"])
if ROM not in ("NXT_ROM_STOCK","NXT_ROM_NEXTUI","NXT_ROM_NXREDUX"):raise ValueError("Invalid ROM")
SDK = ROOT / ".nxt-sdk/tg5040"
CC = SDK / "compiler/bin/aarch64-nextui-linux-gnu-gcc"
TRIPLE = "aarch64-nextui-linux-gnu"
SYSROOT = SDK / ("compiler/" + TRIPLE + "/libc")
BUILD = ROOT / {"NXT_ROM_STOCK":".cache/stock-build","NXT_ROM_NEXTUI":".cache/nextui18-build","NXT_ROM_NXREDUX":".cache/nxredux-build"}[ROM]
OBJ = BUILD / "obj"
BIN = ROOT / {"NXT_ROM_STOCK":"stock/bin/tg5040","NXT_ROM_NEXTUI":"upstream/bin/tg5040","NXT_ROM_NXREDUX":"nxredux/bin/tg5040"}[ROM]
HEADER = BUILD / "headers"

def run(args, **kwargs):
    subprocess.run([str(x) for x in args], check=True, **kwargs)

def archive_headers(filename, destination, strip):
    archive = ROOT / "sources" / filename
    expected = archive.with_name(filename + ".sha256").read_text().split()[0]
    if hashlib.sha256(archive.read_bytes()).hexdigest() != expected:
        raise RuntimeError("Source checksum mismatch: " + filename)
    destination.mkdir(parents=True, exist_ok=True)
    with tarfile.open(archive) as package:
        for member in package.getmembers():
            relative = Path(*Path(member.name).parts[strip:])
            if not member.isfile() or ".." in relative.parts or relative.is_absolute():
                continue
            if not member.name.endswith((".h", ".h.in")):
                continue
            target = destination / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(package.extractfile(member).read())

def prepare_headers():
    for filename, folder, strip in [
        ("ffmpeg-n6.1.6.tar.gz", "ffmpeg", 0),
        ("SDL2_ttf-2.24.0.tar.gz", "sdl-ttf", 1),
        ("freetype-2.13.2.tar.gz", "freetype", 1),
        ("harfbuzz-8.3.0.tar.gz", "harfbuzz", 1),
        ("utf8proc-2.9.0.tar.gz", "utf8proc", 1),
        ("fribidi-1.0.13.tar.xz", "fribidi", 1),
        ("libsamplerate-0.2.2-source.tar.gz", "samplerate", 1),
        ("libzip-1.10.1-source.tar.gz", "zip", 0),
    ]:
        archive_headers(filename, HEADER / folder, strip)
    # The snapshot was built for little-endian AArch64 (unaligned access allowed).
    (HEADER / "ffmpeg/libavutil/avconfig.h").write_text(
        "#ifndef AVUTIL_AVCONFIG_H\n#define AVUTIL_AVCONFIG_H\n"
        "#define AV_HAVE_BIGENDIAN 0\n#define AV_HAVE_FAST_UNALIGNED 1\n#endif\n")
    template = (HEADER / "fribidi/lib/fribidi-config.h.in").read_text()
    replacements = {
        "PACKAGE": "fribidi", "PACKAGE_NAME": "GNU FriBidi",
        "PACKAGE_BUGREPORT": "https://github.com/fribidi/fribidi/issues",
        "FRIBIDI_VERSION": "1.0.13", "FRIBIDI_MAJOR_VERSION": "1",
        "FRIBIDI_MINOR_VERSION": "0", "FRIBIDI_MICRO_VERSION": "13",
        "FRIBIDI_INTERFACE_VERSION": "4", "SIZEOF_INT": "4",
        "FRIBIDI_MSVC_BUILD_PLACEHOLDER": "",
    }
    for key, value in replacements.items():
        template = template.replace("@" + key + "@", value)
    (HEADER / "fribidi/lib/fribidi-config.h").write_text(template)
    sdl = HEADER / "SDL2"
    sdl.mkdir(exist_ok=True)
    shutil.copyfile(HEADER / "sdl-ttf/SDL_ttf.h", sdl / "SDL_ttf.h")
    template = (HEADER / "zip/cmake-zipconf.h.in").read_text()
    values = {"libzip_VERSION": "1.10.1", "libzip_VERSION_MAJOR": "1",
              "libzip_VERSION_MINOR": "10", "libzip_VERSION_PATCH": "1",
              "ZIP_NULLABLE_DEFINES": "#define _Nullable\n#define _Nonnull",
              "LIBZIP_TYPES_INCLUDE": "#include <stdint.h>"}
    for bits in (8, 16, 32, 64):
        values["ZIP_INT" + str(bits) + "_T"] = "int" + str(bits) + "_t"
        values["ZIP_UINT" + str(bits) + "_T"] = "uint" + str(bits) + "_t"
    for key, value in values.items():
        template = template.replace("${" + key + "}", value)
    template = template.replace("#cmakedefine ZIP_STATIC", "#define ZIP_STATIC")
    (HEADER / "zip/lib/zipconf.h").write_text(template)

def main():
    if not CC.exists():
        run(["python3", ROOT / "tools/sdk-assets.py"])
    prepare_headers()
    BIN.mkdir(parents=True, exist_ok=True)
    common = ROOT / "upstream/NextUI/workspace/all/common"
    platform = ROOT / "upstream/NextUI/workspace/tg5040/platform"
    settings = ROOT / "upstream/NextUI/workspace/tg5040/libmsettings"
    src = ROOT / "upstream/src"
    input_folders = [src, common, platform, settings]
    if ROM=="NXT_ROM_NXREDUX":input_folders.append(ROOT / "nxredux/src")
    input_paths = sorted(p for folder in input_folders for p in folder.rglob('*') if p.is_file() and p.suffix in {'.c','.h','.inc'})
    def fingerprint():
        return {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in input_paths}
    input_hashes = fingerprint()
    includes = [HEADER, HEADER / "ffmpeg", HEADER / "utf8proc", HEADER / "zip/lib",
                HEADER / "fribidi/lib", HEADER / "harfbuzz/src",
                HEADER / "freetype/include", HEADER / "samplerate", HEADER / "samplerate/src",
                src, src / "audio", src / "include", src / "include/mbedtls_lib",
                src / "include/yxml",
                src / "include/libogg/include", src / "include/libopus/include",
                src / "include/opusfile/include",
                common, platform, settings,
                ROOT / "upstream/NextUI/workspace/all/minarch/libretro-common/include",
                SYSROOT / "usr/include",
                SYSROOT / "usr/include/SDL2", SYSROOT / "usr/include/dbus-1.0", SYSROOT / "usr/lib/dbus-1.0/include"]
    flags = ["-O2", "-fomit-frame-pointer", "-D_GNU_SOURCE", "-D"+ROM,
             '-DPLATFORM="tg5040"', "-DUSE_SDL2", "-DUSE_GLES",
             "-DGL_GLEXT_PROTOTYPES", "-std=gnu99", "-MMD", "-MP",
             "-Werror=implicit-function-declaration", "-Werror=int-conversion",
             "-Werror=switch", "-include", src / "nxt_i18n.h",
             "-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>"]
    if ROM=="NXT_ROM_NXREDUX":flags.append("-DNXT_ROM_NEXTUI")
    flags += ["-I" + str(path) for path in includes]
    sources = json.loads((ROOT / "stock/frontend-sources.json").read_text())
    objects = []
    def compile_one(relative):
        obj = OBJ / Path(relative).with_suffix(".o")
        obj.parent.mkdir(parents=True, exist_ok=True)
        extra = ["-include", src / "config_readonly_shim.h"] if relative.endswith("/config.c") else []
        run([CC, "-c", *flags, *extra, "-o", obj, ROOT / relative], cwd=src)
        print(ROM+":", Path(relative).name, flush=True)
        return obj
    with ThreadPoolExecutor(max_workers=int(os.environ.get("NXT_JOBS", "4"))) as pool:
        objects = list(pool.map(compile_one, sources))
    settings_objects = []
    settings_sources = [ROOT / "nxredux/src/msettings.c"] if ROM=="NXT_ROM_NXREDUX" else [settings / "msettings.c", common / "displaycal.c"]
    for source in settings_sources:
        obj = BUILD / (source.stem + ".o")
        run([CC, "-c", "-O2", "-fPIC", "-D"+ROM, "-D_GNU_SOURCE",
             "-I" + str(common), "-I" + str(settings), "-o", obj, source])
        settings_objects.append(obj)
    settings_name = "libnxt-nxredux-settings.so" if ROM=="NXT_ROM_NXREDUX" else "libmsettings.so"
    settings_link=[CC,"-shared","-s"]
    if ROM=="NXT_ROM_NXREDUX":settings_link.append("-Wl,-soname,"+settings_name)
    settings_link += ["-o",BIN/settings_name,*settings_objects,"-ltinyalsa","-ldl","-lrt","-lm"]
    if ROM=="NXT_ROM_NXREDUX":settings_link.append("-lasound")
    run(settings_link)
    libs = ROOT / "relink/lib"
    link = [CC, *objects, "-s", "-Wl,--as-needed", "-L" + str(BIN),
            "-L" + str(libs), "-L" + str(SYSROOT / "usr/lib"),
            "-Wl,-rpath-link," + str(SYSROOT / "usr/lib") + ":" + str(SYSROOT / "lib"),
            libs / "libopus.a", libs / "libmbedtls.a"]
    link += [libs / ("ffmpeg/" + name + ".a") for name in
             ["libavformat", "libavcodec", "libswresample", "libavutil"]]
    link += ["-lSDL2_image"]
    link += [libs / ("text/" + name + ".a") for name in
             ["libSDL2_ttf", "libharfbuzz", "libfreetype", "libutf8proc", "libfribidi"]]
    link += ["-lstdc++", "-l:"+settings_name, libs / "libsamplerate.a", libs / "libzip.a",
             "-lsqlite3", "-lm", "-lpthread", "-ldl", "-lz", "-lSDL2",
             "-lGLESv2", "-ldbus-1", "-lfdk-aac", "-lasound", "-o", BIN / "musicplayer.elf"]
    run(link, cwd=src)
    (BIN / "musicplayer.elf").chmod(0o755)
    assert fingerprint() == input_hashes, "Source changed during the ROM build"
    (BIN.parent.parent / ('BUILD_INPUTS_v'+json.loads((ROOT/'SOURCE.json').read_text())['version']+'.json')).write_text(json.dumps({'rom': ROM, 'sources': input_hashes, 'binary_sha256': hashlib.sha256((BIN / 'musicplayer.elf').read_bytes()).hexdigest()}, indent=2)+'\n')
    # Ship the exact objects used for this frontend, rather than older snapshots.
    snapshot = ROOT / {"NXT_ROM_STOCK":"stock/relink/obj","NXT_ROM_NEXTUI":"relink/obj","NXT_ROM_NXREDUX":"nxredux/relink/obj"}[ROM]
    if snapshot.exists():
        shutil.rmtree(snapshot)
    for obj in objects:
        target = snapshot / obj.relative_to(OBJ)
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(obj, target)
    print(ROM+" release built:", BIN / "musicplayer.elf", flush=True)

if __name__ == "__main__":
    main()
