import json
import os
import shutil

import pytest

from quasar.engine_api import solve_one_move


def test_engine_api_cli_fallback():
    spot = {
        "street": "preflop",
        "sb": 1.0,
        "bb": 2.0,
        "ante": 0.0,
        "to_act": 0,
        "button": 1,
        "stacks": [98.0, 100.0],
        "committed_total": [1.0, 2.0],
        "committed_on_street": [1.0, 2.0],
        "last_raise_size": 0.0,
        "board": [],
    }
    # Try to locate CLI binary if built
    candidates = [
        os.path.join(os.path.dirname(os.path.dirname(__file__)), "..", "build", "engine", "quasar_cli"),
        os.path.join(os.getcwd(), "build", "engine", "quasar_cli"),
    ]
    cli = None
    for c in candidates:
        if os.path.exists(c) and os.access(c, os.X_OK):
            cli = c
            break
    if cli is None:
        pytest.skip("quasar_cli not built; skipping CLI fallback test")

    out = solve_one_move(spot, cli_path=cli)
    assert "legal" in out and "uniform_actions" in out
    assert out["legal"]["can_check"] is False
    # Expect facing 1 chip to call SB vs BB
    assert out["legal"]["call_amount"] == 1

