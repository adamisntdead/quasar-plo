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

