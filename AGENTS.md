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
Status: 1–4 completed; 6 partially done (optional C++ Torch wrapper behind flag). Next we focus on solver and evaluation plumbing.

1) Solver skeleton (river-only): CFR node with regret-matching + linear averaging; plug in discretizer; placeholder leaf evaluator.
2) Value-net wrapper test: add C++ test that loads a TorchScript model (when `QUASAR_BUILD_TORCH=ON`) and runs `compute_values` batched.
3) Pybind strategy path: expose `solve_one_move` that returns legal actions + per-action strategy (uniform or CFR-derived), and a Python convenience API.
4) Packing to C++ bridge: define contiguous buffers to pass packed batches to TorchScript from C++; keep behind flag.
5) Bucketing (river): extend features (draw/wrap/flush indicators) and run larger-K clustering for a handful of boards; export hand→bucket maps.
6) Benchmarks: add a micro-benchmark for `solve_one_move` latency (legality + discretization; and with TorchScript if enabled).
7) Documentation: record solver skeleton, value-net query schema, and add/update goldens.

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
Status: Steps 1–6 completed; 7 (CFR) partially implemented (one-step CFR, simple river evaluator); CLI/pybind, packing, bucketing SOP, model stub, Torch wrapper, and tests in place.

1) River tree-CFR (heads-up):
   - Build a river betting tree with discretized pot-limit actions (cap raises 1–2 plys).
   - Information sets keyed by public state + own bucket id; store regrets/avg strategy.
   - Implement CFR over the tree; return root strategy.
2) Equity integration:
   - Extend JSON/pybind to accept per-player bucket distributions and an equity matrix path; add loader and validation.
   - Use K×K equity matrix to compute terminal utilities.
3) solve_one integration:
   - When street=river and ranges+equity are provided, run tree-CFR; otherwise use current evaluator or uniform.
4) Tests and goldens:
   - Add ctests for small fixed board/K; ensure probabilities sum to 1 and no illegal actions.
   - Golden outputs for a fixed config and discretization grid.
5) Benchmarks:
   - Measure latency for river tree-CFR with small K and limited raises; optimize/regress if needed.

## Next Prompt (for tomorrow’s AI)
Implement a heads-up PLO river betting tree and CFR solver over bucketed ranges:
- Add a river tree builder that uses existing pot-limit legality and `DiscretizationConfig` (limit to 1–2 raise rounds).
- Define information sets keyed by (public_state, own_bucket). Maintain regret and average strategy tables.
- Extend SPOT JSON to accept per-player range vectors (`range`: [K] per player) and an `equity_csv` path. Update pybind and CLI to parse these.
- Load K×K equity matrices (CSV) and use them for terminal utilities (check-check and called pots) during CFR.
- Integrate with `solve_one`: when ranges+equity are present and street=river, run tree-CFR and return the root strategy; otherwise keep current behavior.
- Add ctests with a small K (e.g., 3) using `scripts/goldens/equity_K3.csv`, and golden outputs for a fixed spot.
- Add a micro-benchmark measuring solve_one latency with tree-CFR enabled.
