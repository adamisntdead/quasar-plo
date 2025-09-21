#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."/python
python3 -m venv .venv
source .venv/bin/activate
pip install -U pip
pip install -r requirements.txt
pytest -q || true

echo "Virtual env ready at quasar-plo/python/.venv"

