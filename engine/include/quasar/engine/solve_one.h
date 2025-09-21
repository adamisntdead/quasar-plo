#pragma once
#include <vector>

#include "quasar/engine/plo_legal.h"
#include "quasar/engine/rules.h"
#include "quasar/engine/discretize.h"

namespace quasar {

struct SolveOneConfig {
  BettingRules rules;
  DiscretizationConfig discretization;
  int cfr_iters = 0;  // if >0, compute CFR on immediate utilities
};

struct SolveOneResult {
  LegalActionSummary legal;
  std::vector<Action> actions;     // expanded list including check/fold/call
  std::vector<double> probabilities;  // same order as actions
};

SolveOneResult solve_one(const PublicState& s, const SolveOneConfig& cfg);

}  // namespace quasar

