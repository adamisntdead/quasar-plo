#include "quasar/engine/solve_one.h"
#include "quasar/engine/public_state.h"
#include <cassert>
#include <cmath>
#include <iostream>

static inline bool approx(double a, double b, double eps = 1e-4) { return std::fabs(a - b) < eps; }

int main() {
  // River: facing a bet, check evaluations guide CFR behavior.
  quasar::PublicState s;
  s.num_players = 2; s.street = 3; s.sb = 1.0; s.bb = 2.0; s.button = 1; s.player_to_act = 0;
  s.stacks = {100, 100};
  // Opponent bet 4 into pot 10: committed_total: 5+5 pre + opp_on_street=4
  s.committed_total = {5.0, 9.0};
  s.committed_on_street = {0.0, 4.0};
  s.last_raise_size = 4.0;  // voluntary bet size

  quasar::SolveOneConfig cfg;
  cfg.rules = quasar::BettingRules{};
  cfg.discretization = quasar::DiscretizationConfig{};
  cfg.cfr_iters = 200;

  // Case 1: high win prob -> call should get non-trivial mass (>= raise, > fold)
  cfg.eval.win_prob = 0.7; cfg.eval.call_k = 0.5;
  auto res1 = quasar::solve_one(s, cfg);
  // Find probs
  double p_fold = 0, p_call = 0, p_raise = 0;
  for (size_t i = 0; i < res1.actions.size(); ++i) {
    const auto& a = res1.actions[i];
    if ((int)a.type == (int)quasar::ActionType::kFold) p_fold += res1.probabilities[i];
    if ((int)a.type == (int)quasar::ActionType::kCall) p_call += res1.probabilities[i];
    if ((int)a.type == (int)quasar::ActionType::kRaise || (int)a.type == (int)quasar::ActionType::kAllIn) p_raise += res1.probabilities[i];
  }
  assert(p_call > p_fold);

  // Case 2: low win prob -> fold mass should dominate call
  cfg.eval.win_prob = 0.1; cfg.eval.call_k = 0.5;
  auto res2 = quasar::solve_one(s, cfg);
  double p_fold2 = 0, p_call2 = 0;
  for (size_t i = 0; i < res2.actions.size(); ++i) {
    const auto& a = res2.actions[i];
    if ((int)a.type == (int)quasar::ActionType::kFold) p_fold2 += res2.probabilities[i];
    if ((int)a.type == (int)quasar::ActionType::kCall) p_call2 += res2.probabilities[i];
  }
  assert(p_fold2 > p_call2);

  std::cout << "solve_one CFR tests passed" << std::endl;
  return 0;
}

