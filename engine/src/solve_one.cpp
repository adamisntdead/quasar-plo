#include "quasar/engine/solve_one.h"

#include <algorithm>

#include "quasar/solver/cfr.h"
#include "quasar/solver/eval.h"

namespace quasar {

static std::vector<Action> build_action_list(const PublicState& s, const LegalActionSummary& la, const std::vector<Action>& discrete) {
  std::vector<Action> out;
  if (la.can_check) {
    out.push_back({ActionType::kCheck, 0.0});
  } else {
    if (la.can_fold) out.push_back({ActionType::kFold, 0.0});
    out.push_back({ActionType::kCall, la.call_amount});
  }
  out.insert(out.end(), discrete.begin(), discrete.end());
  return out;
}

static std::vector<double> immediate_utilities(const PublicState& s, const LegalActionSummary& la, const std::vector<Action>& actions) {
  std::vector<double> utils;
  utils.reserve(actions.size());
  const int p = s.player_to_act;
  const double me_on_street = s.contributed_this_street(p);
  for (const auto& a : actions) {
    double u = 0.0;
    switch (a.type) {
      case ActionType::kCheck: u = 0.0; break;
      case ActionType::kFold: u = -0.1; break;  // small penalty
      case ActionType::kCall: u = -la.call_amount; break;
      case ActionType::kBet:
      case ActionType::kRaise:
      case ActionType::kAllIn: {
        const double pay = std::max(0.0, a.amount - me_on_street);
        u = -pay;
        break;
      }
      default: u = 0.0; break;
    }
    utils.push_back(u);
  }
  return utils;
}

SolveOneResult solve_one(const PublicState& s, const SolveOneConfig& cfg) {
  auto la = compute_legal_actions(s, cfg.rules);
  auto disc = discretize_actions(s, la, cfg.discretization);
  auto actions = build_action_list(s, la, disc);

  std::vector<double> probs(actions.size(), 0.0);
  if (actions.empty()) {
    return SolveOneResult{la, actions, probs};
  }

  if (cfg.cfr_iters > 0) {
    std::vector<double> utils;
    if (s.street == 3) {
      utils = evaluate_simple_river(s, la, actions, cfg.eval);
    } else {
      utils = immediate_utilities(s, la, actions);
    }
    CFRConfig c;
    c.iters = cfg.cfr_iters;
    auto res = cfr_onestep(utils, c);
    probs = std::move(res.strategy);
  } else {
    const double u = 1.0 / static_cast<double>(actions.size());
    std::fill(probs.begin(), probs.end(), u);
  }

  return SolveOneResult{la, actions, probs};
}

}  // namespace quasar
