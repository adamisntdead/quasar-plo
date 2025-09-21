#pragma once
#include <vector>

#include "quasar/engine/plo_legal.h"

namespace quasar {

struct DiscretizationConfig {
  // Fractions of pot_now to propose as bet/raise-to targets (when feasible)
  std::vector<double> pot_fracs{0.33, 0.5, 0.75, 1.0};
  bool include_min = true;
  bool include_pot_raise = true;  // only when facing a bet
  bool include_all_in = true;
};

// Generate a discrete set of actions from legal bounds.
std::vector<Action> discretize_actions(const PublicState& s,
                                       const LegalActionSummary& la,
                                       const DiscretizationConfig& cfg);

}  // namespace quasar

