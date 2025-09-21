#include "quasar/solver/cfr.h"

#include <algorithm>
#include <numeric>

namespace quasar {

static inline void normalize(std::vector<double>& v) {
  double s = std::accumulate(v.begin(), v.end(), 0.0);
  if (s <= 0) {
    const double u = v.empty() ? 0.0 : 1.0 / static_cast<double>(v.size());
    std::fill(v.begin(), v.end(), u);
  } else {
    for (auto& x : v) x /= s;
  }
}

CFRResult cfr_onestep(const std::vector<double>& action_utils, const CFRConfig& cfg) {
  const int A = static_cast<int>(action_utils.size());
  std::vector<double> regrets(A, 0.0);
  std::vector<double> strategy(A, 0.0);
  std::vector<double> strategy_sum(A, 0.0);

  // Start with uniform strategy
  std::fill(strategy.begin(), strategy.end(), A > 0 ? 1.0 / A : 0.0);

  for (int it = 0; it < cfg.iters; ++it) {
    // Expected utility under current strategy
    double u_sigma = 0.0;
    for (int a = 0; a < A; ++a) u_sigma += strategy[a] * action_utils[a];

    // Regret update and next strategy via regret-matching
    double pos_sum = 0.0;
    for (int a = 0; a < A; ++a) {
      regrets[a] += (action_utils[a] - u_sigma);
      if (regrets[a] > 0) pos_sum += regrets[a];
    }
    if (pos_sum > 0) {
      for (int a = 0; a < A; ++a) strategy[a] = std::max(0.0, regrets[a]) / pos_sum;
    } else {
      std::fill(strategy.begin(), strategy.end(), A > 0 ? 1.0 / A : 0.0);
    }
    // Linear averaging
    for (int a = 0; a < A; ++a) strategy_sum[a] += strategy[a];
  }

  // Average strategy
  normalize(strategy_sum);
  return CFRResult{strategy_sum, regrets};
}

}  // namespace quasar

