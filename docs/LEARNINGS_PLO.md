# Hold’em ➜ PLO: Practical Learnings for a ReBeL‑Style Solver

This document distills implementation learnings from Hold’em ReBeL‑style systems and highlights what changes for Pot‑Limit Omaha (PLO). It focuses on differences from the original ReBeL paper (which assumes manageable belief vectors and simple games) and captures the engineering decisions that make a PLO solver practical, fast, and reproducible.

## 1) Key Differences vs. Original ReBeL Assumptions
- State/Belief dimensionality explodes in PLO
  - Four hole cards (270,725 combos) vs. 1,326 in NLH → hand‑level outputs are infeasible beyond the smallest trees; per‑board bucket outputs are required.
  - Sparse bucketing is not an optimization; it is a necessity (only populated buckets per board are processed).
- Pot‑limit action space
  - The min‑raise and max‑raise depend on current pot and call amount; discrete raise buckets must be derived from a continuous rule and snapped consistently.
- Equity evaluation is heavier
  - Many more draw patterns and deeper wrap/flush interactions; rollouts and caching matter even for leaf evaluation.
- Stability constraints
  - TorchScript inference needs static shapes; all packing and model forwards must avoid dynamic control flow and ragged shapes.

## 2) Action Abstraction: Pot‑Limit Specifics
- Canonical per‑street specs (good starting set)
  - Flop: {50%, 100%, 200% pot, all‑in}, raise cap 3–4.
  - Turn/River: {100% pot, all‑in}, raise cap 3.
- Pot‑limit legality
  - amount_to_call = C; last_raise_size = R; min total raise = C + R; max total bet ≈ C + P + C (i.e., raise by pot + call), capped by stacks.
  - Discrete sizes computed from pot units; always snap to [min, max] and keep rules consistent through recursion and sampling.
- Lookahead vs. sampling
  - Use stricter sets for full solves than for data generation to control branching.

## 3) Information Abstraction (Bucketing)
- Per‑street clustering pipeline
  - Hand descriptors vs. opponent range per board: equity (EHS), second moment E[HS²], made‑hand classes, wrap/straight draw features (e.g., 13‑out wraps), nut/back‑up draw potential, flush potential by suit pattern, blockers, shape (ds/ss/rundown/gappers), optional board texture.
  - Normalize per board; optional PCA; run KMeans/mini‑batch KMeans.
  - Target bucket counts (tune upward over time): flop K∈{1k–4k}; turn K≈1k–2k; river K∈{4k–128k}.
- Suit‑isomorph reduction
  - Canonicalize boards by suit class to reuse mappings and reduce artifacts. Expose transforms hand_iso ↔ hand_canonical.
- Artifacts and contracts
  - Dense: length‑K range and CFV vectors.
  - Sparse: indices (length M ≪ K), range/CFV on those indices, mask, M_max for padding. Prefer sparse model forward to avoid dense scatter at large K.
- Versioning
  - Bucket maps change → checkpoints become incompatible. Version all maps and encode a `bucket_version` in data and models.

## 4) Query Schema, Targets, and Normalization
- Per‑player slice (fixed shape)
  - [PLAYER_ACT, POSITION, S2PR, BOARD[5] as 0..51 with ‑1 padding, RANGE[K] or sparse tuple]. Concatenate two slices → input tensor.
- Target clamping and S2PR scaling
  - Clamp CFVs to [‑0.5, S2PR + 0.5] (loss range), then normalize S2PR features by 100 for model input; unscale consistently in evaluation.
- Safe range normalization
  - Add small ε (≈1e‑32) to avoid divide‑by‑zero; zero out impossible hands/buckets (colliding with board) prior to ingestion.
- Zero‑sum enforcer and EV↔CFV mapping
  - Apply a simple outer‑network correction to enforce zero‑sum at each training example (weighted by ranges), especially helpful on river.
  - If the network predicts EVs for buckets, map EV↔CFV using opponent reaches to train/evaluate interchangeably.

## 5) Solvers and Leaf Evaluation
- CFR/DCFR and FP/BR
  - DCFR discounts stabilize learning at shallow depths; linear averaging for strategies; maintain sampling strategy and belief‑propagation policy.
- Recursive solving
  - Keep depth budget shallow (1–3) for latency; only pseudo‑leaf nodes query the value net; chance nodes can be handled by auxiliary nets (next street aggregation).
- Terminal leaves
  - Exact showdown EVs under beliefs; ensure the same normalization as the pseudo‑leaf net outputs.
- Pseudo‑leaves
  - Query per‑bucket CFVs; multiply by opponent reach sums; scale by the state’s normalize_value_after_nn() to ensure consistency with terminal leaves.
- Equity evaluation engineering
  - Start with CPU evaluator; add Monte Carlo sampling; cache evaluations keyed by (board, ranges); later, add a CUDA evaluator.

## 6) Neural Models and TorchScript
- Dense model (baseline)
  - Input: public features + dense bucketed ranges; optional learned board embeddings; projection → L pre‑norm transformer blocks → linear head to K; optional zero‑sum enforcer.
  - Start with hidden=256, heads=4, layers=2, dropout=0.0.
- Sparse model (for large K)
  - Input: (indices, features). Use index_select to fetch subset rows for input/output embeddings; compute only populated buckets; the head returns values aligned with indices.
- TorchScript constraints
  - Static shapes, no Python‑only logic in forward; freeze and optimize_for_inference; consider FP16 or int8 (post‑training quantization) for CPU.

## 7) Self‑Play, Training, and Evaluation
- Self‑play
  - Random action prob 0.1–0.2; chance save ratio ~10; per‑street lookahead; generator threads per device; push TorchScript snapshots periodically.
- Loss/optimization
  - Huber δ=1 per‑bucket CFV; mask invalid buckets; clamp before S2PR scaling; AdamW LR=3e‑4; warmup ~2k; cosine schedule; grad clip=5.0; batch ~1024.
- Evaluation
  - Approximate exploitability under same abstraction with deeper depth; EV vs. baselines on a suite of public spots; diagnostics: regrets, reach mass, CFV calibration.

## 8) Engineering Pitfalls and Fixes
- Bucketing drift: version maps and embed version in data/models.
- Slow inference at large K: prefer sparse model; pre‑allocate buffers; avoid scatter to dense unless necessary.
- TorchScript breakage: avoid dynamic control flow; test scripting/export paths early.
- Pot‑limit rounding glitches: write golden tests for min/max raise rules and snapping behavior across streets.
- Numerical stability: clamp targets, smooth ranges, compute in FP32 even if inference uses FP16.

---

# Next Agent Plan (Concrete Steps)

1) Engine: extend `PublicState` with last action (actor/type/size), positions, blinds/antes; add pot‑limit min/max raise computation + legal action generator.
2) CLI (or pybind): implement a `solve_one_move` stub that parses a spot JSON and returns legal actions and a uniform strategy distribution.
3) Python transforms: define per‑player slice packing and validation; add unit tests that check shape, scaling, and zeroing rules.
4) Bucketing SOP (river first): implement feature extraction for PLO (equity, E[HS²], wrap/flush indicators, suit/shape), run small‑K clustering (e.g., 500) for a handful of boards, and produce hand→bucket maps.
5) Dense model stub: implement a minimal transformer‑based network; train on synthetic data to validate TorchScript export/import round‑trip.
6) Value‑net interface: add C++ TorchScript wrapper with attributes (input_size, output_size, is_sparse) and a batched `compute_values` call.
7) Solver skeleton: implement CFR with regret‑matching, linear averaging, and a placeholder leaf evaluator; exercise pseudo‑leaf net query path with the dense net stub.
8) Self‑play harness: create a simple generator that samples spots and pushes (query, target) pairs (start with river only) into a replay; train the dense net to reduce MSE on synthetic targets.
9) Evaluation: write a micro-benchmark that measures latency for `solve_one_move` with shallow depth and TorchScript inference; add a dummy exploitability calculator (random opponents) to validate plumbing.
10) Documentation: update docs/DESIGN.md with the legal action math and packing schema; record bucket versioning rules and add golden tests.

---

# One‑Shot Prompt for a Fresh AI

Use this exact prompt to spin up a new AI on this repository:

"""
You are joining the QuasarPLO project (quasar-plo/). Your goals:

1) Extend the C++ engine with pot-limit action legality and a richer PublicState.
- Implement min/max raise rules and a legal action generator per street.
- Add a small CLI or pybind function to print legal actions for a JSON-described spot.

2) Implement Python packing and validation utilities.
- Define a per-player slice: [PLAYER_ACT, POSITION, S2PR, BOARD[5] with -1 padding, RANGE[K] or sparse tuple].
- Add tests that verify shape, S2PR scaling, and zeroing of impossible hands.

3) Start the bucketing SOP for river (small K).
- Implement PLO features (equity, E[HS^2], wrap/flush indicators, suit/shape) and run KMeans on a few boards.
- Emit hand→bucket maps + populated bucket indices; document format.

4) Add a dense TorchScript model stub and export/import path.
- A minimal transformer-based model with input_size/output_size attributes.
- Unit test that C++ can load and compute on batched inputs.

Follow repo docs:
- Read quasar-plo/AGENTS.md for style and contracts.
- Update docs/DESIGN.md with the action legality math and packing once implemented.
Deliver small, testable increments. Do not copy code from other repos; implement from first principles.
"""

