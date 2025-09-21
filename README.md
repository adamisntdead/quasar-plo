# QuasarPLO — A ReBeL‑style Pot‑Limit Omaha Solver (from scratch)

QuasarPLO is a fresh, research‑grade scaffold to build a real‑time Pot‑Limit Omaha (PLO) solver using ReBeL methods: recursive subgame solving with a learned counterfactual value (CFV) function. This repository contains a clean C++ engine skeleton (no code reuse) and a Python research stack for models, data generation, and evaluation.

Goals
- Instant or near‑instant spot solving via shallow recursion + fast inference
- Clear interfaces and contracts (dense/sparse buckets, TorchScript)
- Reproducible research and thesis‑quality documentation

Status: Scaffold (no solver yet). See docs/ for design and roadmap.

## Repo layout

```
quasar-plo/
  AGENTS.md              # Guidance for AI/dev collaborators
  README.md              # This file
  .gitignore
  CMakeLists.txt         # Top-level build entry for C++ engine
  engine/
    CMakeLists.txt
    include/quasar/engine/
      version.h
      public_state.h
    src/
      version.cpp
      public_state.cpp
    pybind/
      CMakeLists.txt
      module.cpp         # Optional pybind11 module (off by default)
  python/
    quasar/
      __init__.py
      models/__init__.py
      transforms/__init__.py
      data_generation/__init__.py
      eval/__init__.py
      configs/README.md
    pyproject.toml
    requirements.txt
    README.md
    tests/test_import.py
  docs/
    DESIGN.md
    ROADMAP.md
  scripts/
    setup_venv.sh
    build_cpp.sh
    format.sh
  tests/
    CMakeLists.txt
    test_version.cpp
```

## Quickstart

- C++ build (engine only):
  ```bash
  cd quasar-plo
  cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
  cmake --build build -j
  ctest --test-dir build
  ```

- Python env:
  ```bash
  cd quasar-plo/python
  python -m venv .venv && source .venv/bin/activate
  pip install -r requirements.txt
  pytest -q
  ```

- Optional pybind module:
  ```bash
  cd quasar-plo
  cmake -S . -B build -DQUASAR_BUILD_PYBIND=ON
  cmake --build build -j
  ```

## CLI: Legal actions

Build target `quasar_cli` prints legal actions for a JSON-described spot:

```
build/engine/quasar_cli scripts/example_spot.json
```

Schema (fields used):
- `street`: "preflop"|"flop"|"turn"|"river"
- `sb`, `bb`, `ante`: numbers
- `to_act`, `button`: integers
- `stacks`: [N floats]
- `committed_total`: [N floats]
- `committed_on_street`: [N floats]
- `last_raise_size`: float (optional)
- `board`: [0..51] (optional)

See docs/SPOT_JSON.md for the full request/response schema, including optional
`min_bet_rule` and `discretization` configuration.

## Benchmarks
- Build: `build/engine/quasar_bench`
- Usage: `build/engine/quasar_bench scripts/example_spot.json 20000`
- Prints average microseconds per `solve_one` call (legality + discretization, optional CFR).

## Pybind structured API
When built with `-DQUASAR_BUILD_PYBIND=ON`, the module exposes:
- `solve_one_move_json(json_str) -> json_str` (for CLI parity)
- `solve_one_move(json_str) -> (actions, probs, legal_dict)`

Python convenience wrapper:
- `python/quasar/engine_api.py` provides `solve_one_move(spot, cli_path=None)` which uses pybind if available, else falls back to the CLI.

## Next steps
- Implement poker/PLO public state + action legality (pot‑limit math) in `engine/`
- Add tree + recursive solver (CFR/DCFR; FP/BR) skeletons
- Add dense/sparse TorchScript model stubs and packing in `python/quasar/`
- Define configs and a “solve one spot” CLI (JSON in/out)

See docs/DESIGN.md and docs/ROADMAP.md for details.
