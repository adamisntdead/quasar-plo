#include "quasar/engine/discretize.h"

#include <algorithm>
#include <cmath>

namespace quasar {

static inline double clamp(double x, double lo, double hi) {
  return std::max(lo, std::min(hi, x));
}

std::vector<Action> discretize_actions(const PublicState& s,
                                       const LegalActionSummary& la,
                                       const DiscretizationConfig& cfg) {
  std::vector<Action> out;
  const double pot_now = s.pot_total();
  auto unique_push = [&](ActionType t, double amt) {
    for (const auto& a : out) {
      if (a.type == t && std::fabs(a.amount - amt) < 1e-9) return;
    }
    out.push_back({t, amt});
  };

  if (la.bet_bounds) {
    const double lo = la.bet_bounds->min_to;
    const double hi = la.bet_bounds->max_to;
    if (cfg.include_min) unique_push(ActionType::kBet, lo);
    for (double f : cfg.pot_fracs) {
      double to = clamp(f * pot_now, lo, hi);
      unique_push(ActionType::kBet, to);
    }
    if (cfg.include_all_in && hi > lo + 1e-9) unique_push(ActionType::kAllIn, hi);
  }
  if (la.raise_bounds) {
    const double lo = la.raise_bounds->min_to;
    const double hi = la.raise_bounds->max_to;
    if (cfg.include_min) unique_push(ActionType::kRaise, lo);
    if (cfg.include_pot_raise) {
      // pot raise target was: current_bet + pot_after_call
      const double max_bet = s.current_bet_to_call();
      const double pot_raise_to = clamp(max_bet + pot_after_call(s, s.player_to_act), lo, hi);
      unique_push(ActionType::kRaise, pot_raise_to);
    }
    for (double f : cfg.pot_fracs) {
      const double max_bet = s.current_bet_to_call();
      double to = clamp(max_bet + f * pot_after_call(s, s.player_to_act), lo, hi);
      unique_push(ActionType::kRaise, to);
    }
    if (cfg.include_all_in && hi > lo + 1e-9) unique_push(ActionType::kAllIn, hi);
  }
  return out;
}

}  // namespace quasar

