#!/bin/bash
set -euo pipefail
NXT_QA_ROOT=$(cd -- "$(dirname -- "$0")/.." && pwd)
export LD_LIBRARY_PATH="$NXT_QA_ROOT/.host-tools/usr/lib/x86_64-linux-gnu"
exec "$NXT_QA_ROOT/.host-tools/usr/bin/qemu-aarch64" -L "$NXT_QA_ROOT/.cache/stock-research/rootfs" -E 'LD_LIBRARY_PATH=/usr/trimui/lib:/usr/lib:/lib' "$@"
