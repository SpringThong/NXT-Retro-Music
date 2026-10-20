#!/bin/bash
set -euo pipefail
NXT_SPOTIFY_APP_ROOT=$(cd -- "$(dirname -- "$0")/.." && pwd)
export NXT_SPOTIFY_SDK="${NXT_SPOTIFY_SDK:-$NXT_SPOTIFY_APP_ROOT/.nxt-sdk/tg5040/compiler}"
export RUSTUP_HOME="${RUSTUP_HOME:-$HOME/.rustup}"
export CARGO_HOME="${CARGO_HOME:-$HOME/.cargo}"
bash "$NXT_SPOTIFY_APP_ROOT/spotify/tools/build-backend.sh"
NXT_SPOTIFY_TARGET_DIR="${CARGO_TARGET_DIR:-$NXT_SPOTIFY_APP_ROOT/spotify/backend/target}"
cp "$NXT_SPOTIFY_TARGET_DIR/aarch64-unknown-linux-gnu/release/nxt-spotify-backend" "$NXT_SPOTIFY_APP_ROOT/upstream/bin/tg5040/nxt-spotify-backend"
chmod +x "$NXT_SPOTIFY_APP_ROOT/upstream/bin/tg5040/nxt-spotify-backend"
python3 - "$NXT_SPOTIFY_APP_ROOT" <<'PY'
from pathlib import Path
import hashlib, json, shutil, sys
root = Path(sys.argv[1])
manifest_path = root / "overlay-manifest.json"
manifest = json.loads(manifest_path.read_text())
relative = "bin/tg5040/nxt-spotify-backend"
source = root / "upstream" / relative
for item in manifest["files"]:
    if item["path"] == relative:
        shutil.copy2(source, root / "overlay" / relative)
        item["sha256"] = hashlib.sha256(source.read_bytes()).hexdigest()
        break
else:
    raise SystemExit("Spotify helper missing from overlay manifest")
manifest_path.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + "\n")
PY
