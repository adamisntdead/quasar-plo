# AGENTS.md — Guidance for AI/dev collaborators

Scope: This file governs agent behavior across the entire `quasar-plo/` repo. Follow these rules for consistency and velocity.

## Mission
Build a from-scratch, ReBeL-style Pot-Limit Omaha solver capable of near real-time spot solving. Code must be original (no copy-paste from prior repos), but design may be inspired by public research.

## Architecture overview
- `engine/` (C++17/20)
  - Game model (PLO), action legality (pot-limit), public state, tree.
  - Solvers: CFR/DCFR and FP/BR; recursive solving; strategy averaging.
  - `nn/` interface for TorchScript value-nets (dense/sparse signatures).
  - Optional pybind11 module (behind `QUASAR_BUILD_PYBIND`).
- `python/` (research)
  - Model definitions, TorchScript export, bucketing/transforms, self-play/training, eval.
- `docs/` design notes, roadmap.

## Code style & tooling
- C++: C++17 or newer. Header-only includes under `engine/include/quasar/...`. Prefer `std::unique_ptr`, `std::shared_ptr` judiciously. No exceptions across C/Python boundary; propagate errors as statuses where needed.
- Build: CMake (>=3.18). Keep `QUASAR_BUILD_PYBIND` optional. No external network fetches inside CMake.
- Python: Python 3.10+. Use type hints where reasonable. Keep TorchScript-exportable forwards (no dynamic control flow, static shapes). Black/ruff formatting preferred.

## Contracts
- Value-net (C++)
  - `compute_values(queries[, indices]) -> Tensor[batch, players, out_size]`
  - Attributes: `input_size:int`, `output_size:int`, `is_sparse:bool`
- Packing (Python)
  - Per-player slice: `[PLAYER_ACT, POSITION, S2PR, BOARD[5], RANGE[K]]` or sparse `(indices, features, mask)`.
  - Targets clamp: `[-0.5, S2PR + 0.5]`; S2PR scaled by 100 post-clamp.

## What to do next
1) Implement pot-limit action legality and a minimal `PublicState` in C++.
2) Add a tiny CLI/API to dump legal actions for a given spot.
3) Prototype bucketing features and a small KMeans pipeline for river (small K).
4) Add a dense net stub and TorchScript export; verify C++ can load and call it.
5) Integrate a skeletal CFR loop with pseudo-leaf net queries (no recursion yet).

## Don’ts
- Don’t copy code from prior repos.
- Don’t add heavy deps or network fetches in build.
- Don’t introduce dynamic shapes or Python-only logic into TorchScripted models.

## Testing
- C++: add `tests/` with `ctest` targets; start with version + state sanity.
- Python: `pytest` for import and basic transforms. Keep fast.

## Performance notes
- Pre-allocate buffers; avoid per-call heap allocs in hot paths.
- Prefer contiguous memory for tensors. Consider FP16 in generators.
- Sparse nets for large bucket counts.

## Communication
- Keep README and docs updated when interfaces change.
- Document assumptions in `docs/DESIGN.md` before major refactors.

## Next Agent Plan (Concrete Steps)
1) Engine: extend `PublicState` with last action (actor/type/size), positions, blinds/antes; add pot‑limit min/max raise computation + legal action generator.
2) CLI (or pybind): implement a `solve_one_move` stub that parses a spot JSON and returns legal actions and a uniform strategy distribution.
3) Python transforms: define per‑player slice packing and validation; add unit tests that check shape, scaling, and zeroing rules.
4) Bucketing SOP (river first): implement feature extraction for PLO (equity, E[HS^2], wrap/flush indicators, suit/shape), run small‑K clustering (e.g., 500) for a handful of boards, and produce hand→bucket maps.
5) Dense model stub: implement a minimal transformer‑based network; train on synthetic data to validate TorchScript export/import round‑trip.
6) Value‑net interface: add C++ TorchScript wrapper with attributes (input_size, output_size, is_sparse) and a batched `compute_values` call.
7) Solver skeleton: implement CFR with regret‑matching, linear averaging, and a placeholder leaf evaluator; exercise pseudo‑leaf net query path with the dense net stub.
8) Self‑play harness: create a simple generator that samples spots and pushes (query, target) pairs (start with river only) into a replay; train the dense net to reduce MSE on synthetic targets.
9) Evaluation: write a micro-benchmark that measures latency for `solve_one_move` with shallow depth and TorchScript inference; add a dummy exploitability calculator (random opponents) to validate plumbing.
10) Documentation: update docs/DESIGN.md with the legal action math and packing schema; record bucket versioning rules and add golden tests.
