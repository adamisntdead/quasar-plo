# Configs

Place YAML/JSON configs here for:
- Model hyperparameters (per street)
- Solver settings (CFR/DCFR params, depth, iterations)
- Self-play generation (random action prob, lookahead sizes)

Example sections to include:
- model: {type: dense|sparse, hidden, layers, heads, dropout, zero_sum}
- buckets: {K, keep_board, sparse: {M_max}}
- solver: {dcfr: true, alpha, beta, gamma, depth, iters}
- selfplay: {random_action_prob, chance_save_ratio, lookahead_sizes}

