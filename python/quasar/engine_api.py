from __future__ import annotations

import json
import os
import shutil
import subprocess
from typing import Any, Dict, Optional, Union


def _to_json_str(data: Union[str, Dict[str, Any]]) -> str:
    if isinstance(data, str):
        return data
    return json.dumps(data)


def solve_one_move(spot: Union[str, Dict[str, Any]], *, cli_path: Optional[str] = None) -> Dict[str, Any]:
    """Solve one move by calling pybind if available, else falling back to CLI.

    Args:
        spot: dict or JSON string matching docs/SPOT_JSON.md schema
        cli_path: optional path to compiled quasar_cli binary; if None, tries to
                  discover in common build locations.
    Returns:
        Parsed JSON dict with keys: legal, uniform_actions
    """
    payload = _to_json_str(spot)
    # Try pybind first
    try:
        import quasar_engine_py as qepy  # type: ignore

        out = qepy.solve_one_move_json(payload)
        return json.loads(out)
    except Exception:
        pass

    # Fallback to CLI
    if cli_path is None:
        # try standard build path
        candidates = [
            os.path.join(os.path.dirname(os.path.dirname(__file__)), "..", "build", "engine", "quasar_cli"),
            os.path.join(os.getcwd(), "build", "engine", "quasar_cli"),
        ]
        for c in candidates:
            if os.path.exists(c) and os.access(c, os.X_OK):
                cli_path = c
                break
    if cli_path is None:
        raise RuntimeError("CLI not found; set cli_path to quasar_cli")

    proc = subprocess.run([cli_path], input=payload.encode("utf-8"), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if proc.returncode != 0:
        raise RuntimeError(f"quasar_cli failed: {proc.stderr.decode('utf-8', 'ignore')}")
    return json.loads(proc.stdout.decode("utf-8"))

