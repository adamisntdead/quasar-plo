Spot JSON Schema (CLI/Pybind)

This document describes the minimal JSON schema accepted by the CLI (`quasar_cli`) and the pybind entry (`solve_one_move_json`).

Required fields
- street: "preflop" | "flop" | "turn" | "river"
- sb: number (small blind)
- bb: number (big blind)
- ante: number (equal ante for all)
- to_act: integer (player index to act)
- button: integer (dealer index)
- stacks: [float] per-player stacks behind
- committed_total: [float] per-player total committed (all streets)
- committed_on_street: [float] committed on current street

Optional fields
- last_raise_size: float (size of last raise/bet on this street)
- board: [int] up to 5 cards (0..51), missing entries are padded with -1 internally
- min_bet_rule: "BigBlind" (default) | "OneChip"
- discretization: object controlling discrete action set generation
  - pot_fracs: [float] list of pot fractions for bet/raise-to suggestions (default [0.33, 0.5, 0.75, 1.0])
  - include_min: bool (default true)
  - include_pot_raise: bool (default true)
  - include_all_in: bool (default true)

Response JSON
- legal: { can_check: bool, can_fold: bool, call_amount: float, bet?: {min_to,max_to}, raise?: {min_to,max_to}, suggestions: [ {type:int, amount:float} ] }
- uniform_actions: [ {type:int, amount:float, prob:float} ] — equal-prob discrete set built from check/fold/call and discretized suggestions

Action types
- 0=Fold, 1=Check, 2=Call, 3=Bet, 4=Raise, 5=AllIn

Notes
- All amounts are in chip units consistent with inputs.
- Pot-limit math follows docs/DESIGN.md with a parameterizable min-bet rule.
- Discretization only suggests targets within legal [min_to, max_to].

