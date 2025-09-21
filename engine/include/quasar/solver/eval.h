#pragma once
#include <vector>

#include "quasar/engine/public_state.h"
#include "quasar/engine/plo_legal.h"
#include "quasar/engine/solve_one.h"

namespace quasar {

// Simple river-only evaluator returning per-action utilities.
// Heuristic: models opponent fold/call probability as a function of sizing and
// uses a fixed showdown win probability for call/raise-call outcomes.
std::vector<double> evaluate_simple_river(const PublicState& s,
                                          const LegalActionSummary& la,
                                          const std::vector<Action>& actions,
                                          const SolveOneConfig::SimpleEvalConfig& cfg);

}  // namespace quasar

