#!/bin/bash
set -euo pipefail
NXT_SPOTIFY_ROOT=$(cd -- "$(dirname -- "$0")/.." && pwd)
export RUSTUP_HOME="${RUSTUP_HOME:-$NXT_SPOTIFY_ROOT/toolchain/rustup}"
export CARGO_HOME="${CARGO_HOME:-$NXT_SPOTIFY_ROOT/toolchain/cargo}"
export PATH="$CARGO_HOME/bin:$PATH"
NXT_SPOTIFY_SDK=${NXT_SPOTIFY_SDK:-$NXT_SPOTIFY_ROOT/../.nxt-sdk/tg5040/compiler}
export CARGO_TARGET_AARCH64_UNKNOWN_LINUX_GNU_LINKER="$NXT_SPOTIFY_SDK/bin/aarch64-nextui-linux-gnu-gcc"
export CC_aarch64_unknown_linux_gnu="$CARGO_TARGET_AARCH64_UNKNOWN_LINUX_GNU_LINKER"
export AR_aarch64_unknown_linux_gnu="$NXT_SPOTIFY_SDK/bin/aarch64-nextui-linux-gnu-ar"
export PKG_CONFIG_ALLOW_CROSS=1
export PKG_CONFIG_SYSROOT_DIR="$NXT_SPOTIFY_SDK/aarch64-nextui-linux-gnu/libc"
export PKG_CONFIG_LIBDIR="$PKG_CONFIG_SYSROOT_DIR/usr/lib/pkgconfig"
chmod +x "$NXT_SPOTIFY_ROOT/tools/alsa-pkg-config.py"
if [ -z "${PKG_CONFIG:-}" ] && ! command -v pkg-config >/dev/null; then export PKG_CONFIG="$NXT_SPOTIFY_ROOT/tools/alsa-pkg-config.py"; fi
cargo build --locked --manifest-path "$NXT_SPOTIFY_ROOT/backend/Cargo.toml" --release --target aarch64-unknown-linux-gnu
