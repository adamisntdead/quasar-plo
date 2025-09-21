#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo "$@"
cmake --build build -j
ctest --test-dir build --output-on-failure || true

