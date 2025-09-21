#pragma once
#include <vector>

namespace quasar {

struct CFRConfig {
  int iters = 100;
};

struct CFRResult {
  std::vector<double> strategy;  // normalized average strategy
  std::vector<double> regrets;   // final regrets
};

// One-step CFR at a single decision node for a single player.
// action_utils: utility per action for the acting player (higher is better).
CFRResult cfr_onestep(const std::vector<double>& action_utils, const CFRConfig& cfg = {});

}  // namespace quasar

