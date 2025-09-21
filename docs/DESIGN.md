# QuasarPLO — Design Notes

This document captures the system design, key interfaces, and technical choices for a from-scratch ReBeL-style PLO solver.

## Objectives
- Instant or near-instant solving per spot
- Modular engine with clean contracts (value-net, solvers, tree)
- Dense/sparse information abstraction with TorchScript-ready models

## High-level Architecture
- Engine (C++): PLO game model, action legality (pot-limit), public tree, CFR/DCFR + FP/BR solvers, leaf evaluation API, value-net interface (TorchScript wrapper).
- Python: Bucketing generator (features & clustering), packing transforms, neural models (dense/sparse), TorchScript export, self-play orchestration, evaluation.

## Contracts (must-have)
- Value-net: `compute_values(queries[, indices]) -> [batch, players, out_size]`, attributes `input_size`, `output_size`, `is_sparse`.
- Per-player slice: `[PLAYER_ACT, POSITION, S2PR, BOARD[5], RANGE[K]]` (or sparse tuple) with stable normalization.

## Open Questions / Decisions
- Exact K per street (scaling roadmap)
- Equity evaluator for PLO rollouts (CPU first, CUDA later)
- Quantization path for CPU inference

See ROADMAP.md for milestones.

## Pot-Limit Action Legality (Math)
- Let `atc = amount_to_call(player)` for the player to act in the current betting round.
- Let `pot_now = sum(committed_total)` which includes current street contributions (uncalled bets included).
- If there is no live bet (`atc == 0`):
  - Min bet: `max(bb, 0)` (capped by stack). Parameterizable via rules: `min_bet_rule = BigBlind | OneChip`.
  - Max bet: `min(pot_now, stack)`
- If facing a live bet (`atc > 0`):
  - Min raise size: `last_raise_size` if a voluntary raise/bet has occurred on this street; otherwise `bb` (e.g., preflop blinds).
  - Min raise target: `current_bet + min_raise_size`.
  - Pot after call: `pot_after_call = pot_now + atc`.
  - Max raise over call: `min(pot_after_call, stack - atc)`.
  - Max raise target: `current_bet + max_raise_over_call`.

Notes:
- We track per-player `committed_on_street` and `committed_total`, and `current_bet = max(committed_on_street)`.
- The "target" is the new bet level on this street; the amount a raiser pays this action is `(target - committed_on_street[player])`.

## Packing Schema (Dense)
Per-player slice `[PLAYER_ACT, POSITION, S2PR, BOARD[5], RANGE[K]]` where:
- `PLAYER_ACT`: 1 if this seat acts, else 0
- `POSITION`: seat index relative to Button (BTN=0, clockwise)
- `S2PR`: stack-to-pot ratio; we clamp targets to `[-0.5, S2PR+0.5]` and scale by `100`
- `BOARD[5]`: integer cards (0..51), padded with `-1`
- `RANGE[K]`: probability vector over K bucketed hands; impossible hands are zeroed (share any board card)

## River Bucketing (SOP)
- Features (placeholder, deterministic): hand suit shape (4), top-2 ranks (2), board suit multiplicity (1), board top rank multiplicity (1) => D=8.
- Clustering: basic KMeans (numpy) with random init, small K (<=500).
- Outputs:
  - `labels`: `[N]` hand→bucket id for input order
  - `centers`: `[K, D]` cluster centers
- `features`: `[N, D]` pre-cluster feature matrix

## River Solver (Plan)
- Goal: full PLO river solver (heads-up initially) to bootstrap training.
- Components:
  - Betting tree builder (river-only): nodes capture pot, stacks, to_act, last_raise_size; edges from discretized pot-limit actions until terminal (check/check, call, fold).
  - Information sets: keyed by public state + own bucket id. Maintain regrets and average strategies per info set.
  - Terminal utilities: showdown outcomes via exact evaluator, or bucket-vs-bucket expected utility matrix if bucketing is used.
  - CFR loop: regret-matching updates + linear averaging over iterations; export root strategy.
  - Performance: cache legal action sets per state; reuse pot-limit math; precompute bucket equity matrices per board.

Near-term steps:
1) Implement a PLO river hand evaluator (exact 5-card hand ranks; select 2-of-4 + 3-of-5), with tests.
2) Add bucket-vs-bucket equity precomputation for river boards; produce KxK matrices and a loader.
3) Build a river betting tree (heads-up) with discretized actions; add a CFR solver over this tree with info-set averaging.
4) Integrate into `solve_one` to return root strategy; add ctests/goldens and micro-benchmarks.

## PLO River Evaluator (Exact)
- 5-card evaluator ranks hands by category: High Card < Pair < Two Pair < Trips < Straight < Flush < Full House < Quads < Straight Flush.
- Cards use indices 0..51 with rank = c % 13 (2..A→0..12) and suit = c / 13.
- PLO river best hand selects exactly 2 hole cards and 3 board cards; we enumerate 6 hole combos x 10 board combos and evaluate all 60 five-card hands.
- Deterministic key encodes category and tie-breakers to allow fast max/compare.

## Equity Matrix Format (River Buckets)
- CSV file with K rows, K columns; floats are row-player expected showdown result vs column-player (diagonal=1.0).
- Comments (#) and empty lines are ignored; both commas and spaces are supported as separators.
- Loader: `EquityMatrix` with `K` and `data` (row-major), see engine/include/quasar/solver/equity_matrix.h.
- Discretization Grid (for solver action sets)
  - Configurable pot-fraction grid `pot_fracs` (default `[0.33, 0.5, 0.75, 1.0]`)
  - Include min-raise/min-bet, pot-raise (when facing bet), and all-in flags
  - See `DiscretizationConfig` and `discretize_actions()` in engine
