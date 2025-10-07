#!/usr/bin/env bash
# Wrapper script to run benchmark with Python 3.10 via uv

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ $# -ne 1 ]; then
    echo "Usage: $0 <input_file>" >&2
    exit 1
fi

cd "$SCRIPT_DIR"
uv run --python 3.10 python_bench.py "$1"
