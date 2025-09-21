# QuasarPLO — Roadmap

Milestones
- M0 (2 wks): Engine skeleton + public state + action legality stub + tests
- M1 (4 wks): Tree + action abstraction + hand evaluator hook
- M2 (6 wks): Bucketing generator (river, small K) + dense net + TorchScript
- M3 (9 wks): CFR/DCFR solver + recursive solving + self-play loop
- M4 (12 wks): Sparse nets + increase K; exploitability evaluation suite
- M5 (16 wks): Real-time solver prototype; ablations and thesis figures

Key risks & mitigations
- Memory/latency at high K → sparse nets, FP16, pre-alloc, minimal copies
- Bucket quality → richer features, stratified board sampling, min cluster sizes
- Equity eval cost → caching, sampling strategies, CUDA path later

