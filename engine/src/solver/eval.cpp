#include "quasar/solver/eval.h"

#include <algorithm>
#include <cmath>

namespace quasar {

static inline double clamp(double x, double lo, double hi) { return std::max(lo, std::min(hi, x)); }

std::vector<double> evaluate_simple_river(const PublicState& s,
                                          const LegalActionSummary& la,
                                          const std::vector<Action>& actions,
                                          const SolveOneConfig::SimpleEvalConfig& cfg) {
  std::vector<double> utils;
  utils.reserve(actions.size());
  const int p = s.player_to_act;
  const int q = 1 - p;  // assume heads-up
  const double pot_now = s.pot_total();
  const double me_on = s.contributed_this_street(p);
  const double opp_on = s.contributed_this_street(q);
  const double max_bet = s.current_bet_to_call();
  const bool facing_bet = la.call_amount > 0.0 && (max_bet > me_on + 1e-12);

  auto call_prob_from_size = [&](double to_amount) {
    // Size as fraction of pot; higher leads to lower call prob.
    const double denom = std::max(1e-9, pot_now);
    const double frac = clamp(to_amount / denom, 0.0, 4.0);  // cap to avoid extremes
    double call_prob = clamp(1.0 - cfg.call_k * frac, 0.05, 0.95);
    return call_prob;
  };

  for (const auto& a : actions) {
    double u = 0.0;
    switch (a.type) {
      case ActionType::kCheck: {
        // Neutral baseline (no more money goes in)
        u = 0.0;
        break;
      }
      case ActionType::kFold: {
        // Small penalty: folding surrenders claim to pot; scale light to avoid degenerate behavior
        u = -0.1;
        break;
      }
      case ActionType::kCall: {
        // EV(call) ≈ -call + w * (pot_now + call)
        const double pay = la.call_amount;
        const double win = cfg.win_prob * (pot_now + pay);
        u = -pay + win;
        break;
      }
      case ActionType::kBet: {
        // EV(bet to T) ≈ f_fold * (pot_now) + (1 - f_fold) * (-T + w * (pot_now + 2T))
        const double T = a.amount;
        const double f_call = call_prob_from_size(T);
        const double f_fold = 1.0 - f_call;
        u = f_fold * (pot_now) + (1.0 - f_fold) * (-T + cfg.win_prob * (pot_now + 2.0 * T));
        break;
      }
      case ActionType::kRaise:
      case ActionType::kAllIn: {
        // EV(raise to T) when facing bet:
        // pay = T - me_on; if opp folds: win pot_now + pay; if opp calls: -pay + w * (pot_now + pay + (T - opp_on))
        const double T = a.amount;
        const double pay = std::max(0.0, T - me_on);
        const double opp_call_amt = std::max(0.0, T - opp_on);
        const double f_call = call_prob_from_size(T);
        const double f_fold = 1.0 - f_call;
        const double win_fold = pot_now + pay;
        const double win_call = -pay + cfg.win_prob * (pot_now + pay + opp_call_amt);
        u = f_fold * win_fold + f_call * win_call;
        break;
      }
      default: u = 0.0; break;
    }
    utils.push_back(u);
  }
  return utils;
}

}  // namespace quasar

