#!/usr/bin/env bash
set -euo pipefail
NXT_SC_ROOT=$(cd -- "$(dirname -- "$0")/.." && pwd)
NXT_SC_SDK="${NXT_SC_SDK:-$NXT_SC_ROOT/.nxt-sdk/tg5040/compiler}"
export CARGO_TARGET_AARCH64_UNKNOWN_LINUX_GNU_LINKER="$NXT_SC_SDK/bin/aarch64-nextui-linux-gnu-gcc"
export CC_aarch64_unknown_linux_gnu="$CARGO_TARGET_AARCH64_UNKNOWN_LINUX_GNU_LINKER"
export AR_aarch64_unknown_linux_gnu="$NXT_SC_SDK/bin/aarch64-nextui-linux-gnu-ar"
NXT_SC_TARGET="${CARGO_TARGET_DIR:-$NXT_SC_ROOT/soundcloud/backend/target}"
cargo build --locked --manifest-path "$NXT_SC_ROOT/soundcloud/backend/Cargo.toml" --release --target aarch64-unknown-linux-gnu
cp "$NXT_SC_TARGET/aarch64-unknown-linux-gnu/release/nxt-soundcloud-backend" "$NXT_SC_ROOT/upstream/bin/tg5040/nxt-soundcloud-backend"
"$NXT_SC_SDK/bin/aarch64-nextui-linux-gnu-strip" "$NXT_SC_ROOT/upstream/bin/tg5040/nxt-soundcloud-backend"
chmod +x "$NXT_SC_ROOT/upstream/bin/tg5040/nxt-soundcloud-backend"
python3 "$NXT_SC_ROOT/tools/sync-overlay.py"
