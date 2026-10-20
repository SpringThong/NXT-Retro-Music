#!/bin/sh
set -eu
DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
SYSTEM="${SYSTEM_PATH:-${SDCARD_PATH:-/mnt/SDCARD}/.system}"
TARGET="$SYSTEM/bin/nextui.elf"
BACKUP="$DIR/backup/nextui-v2.1.2-original.elf"
ORIGINAL=ded138923424aa6ed324382bfc1c1905af3989002ea7376afcd58d4da8c2720c
PATCHED=ea1f92faab6f6efc0df6fb6b483a41a7979e888bd1a532cba89fb1786dd5ad47
hash() { sha256sum "$1" | awk '{print $1}'; }
[ -f "$TARGET" ] && command -v sha256sum >/dev/null 2>&1 || exit 1
CURRENT=$(hash "$TARGET")
if [ "${1:-}" = --restore ]; then
    [ "$CURRENT" = "$PATCHED" ] && [ -f "$BACKUP" ] && [ "$(hash "$BACKUP")" = "$ORIGINAL" ] || exit 1
    cp "$BACKUP" "$TARGET.nxt-restore"; chmod 755 "$TARGET.nxt-restore"; mv "$TARGET.nxt-restore" "$TARGET"
    rm -f "$SYSTEM/res/menu/menu_icon_nxt.png"
    printf '%s\n' 'NX Redux launcher restored.'
    exit 0
fi
[ "$CURRENT" = "$ORIGINAL" ] || [ "$CURRENT" = "$PATCHED" ] || { printf '%s\n' 'Tools icon integration skipped: launcher differs from official NX Redux v2.1.2.'; exit 1; }
[ "$(hash "$DIR/nextui-v2.1.2-nxt-icon.elf")" = "$PATCHED" ] || exit 1
mkdir -p "$DIR/backup" "$SYSTEM/res/menu"
if [ "$CURRENT" = "$ORIGINAL" ]; then
    if [ -f "$BACKUP" ]; then [ "$(hash "$BACKUP")" = "$ORIGINAL" ] || exit 1
    else cp "$TARGET" "$BACKUP.tmp"; [ "$(hash "$BACKUP.tmp")" = "$ORIGINAL" ] || exit 1; mv "$BACKUP.tmp" "$BACKUP"; fi
fi
cp "$DIR/menu_icon_nxt.png" "$SYSTEM/res/menu/menu_icon_nxt.png.tmp"
mv "$SYSTEM/res/menu/menu_icon_nxt.png.tmp" "$SYSTEM/res/menu/menu_icon_nxt.png"
if [ "$CURRENT" = "$ORIGINAL" ]; then
    cp "$DIR/nextui-v2.1.2-nxt-icon.elf" "$TARGET.nxt-install"
    chmod 755 "$TARGET.nxt-install"
    [ "$(hash "$TARGET.nxt-install")" = "$PATCHED" ] || { rm -f "$TARGET.nxt-install"; exit 1; }
    mv "$TARGET.nxt-install" "$TARGET"
fi
printf '%s\n' 'NXT Tools icon ready. Reopen Tools or restart the device to refresh the launcher.'

