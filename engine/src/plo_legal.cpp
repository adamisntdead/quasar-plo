#include "quasar/engine/plo_legal.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace quasar {

static inline double clamp(double x, double lo, double hi) {
  return std::max(lo, std::min(hi, x));
}

double pot_after_call(const PublicState& s, int player) {
  const double atc = s.amount_to_call(player);
  return s.pot_total() + atc;
}

double min_raise_size(const PublicState& s) {
  // If there has been a voluntary raise/bet this street, min raise size is the
  // last raise size; otherwise, default to big blind.
  if (s.last_raise_size > 0) return s.last_raise_size;
  return s.bb;
}

LegalActionSummary compute_legal_actions(const PublicState& s, const BettingRules& rules) {
  LegalActionSummary out;
  const int p = s.player_to_act;
  const double atc = s.amount_to_call(p);
  const double max_bet = s.current_bet_to_call();
  const double me_on_street = s.contributed_this_street(p);
  const double my_stack = (p < static_cast<int>(s.stacks.size())) ? s.stacks[p] : 0.0;
  const double pot_now = s.pot_total();

  const bool facing_bet = atc > 0.0 && (max_bet > me_on_street + 1e-12);
  out.can_check = !facing_bet;
  out.can_fold = facing_bet && my_stack > 0.0;
  out.call_amount = std::min(atc, my_stack);

  if (!facing_bet) {
    // Bet case
    const double base_min = (rules.min_bet_rule == BettingRules::MinBetRule::BigBlind) ? s.bb : 1.0;
    const double min_to = std::min(std::max(base_min, 0.0), my_stack);
    const double max_to = std::min(pot_now, my_stack);
    if (max_to >= min_to + 1e-9) {
      out.bet_bounds = RaiseBounds{min_to, max_to};
    }
  } else {
    // Raise case
    const double min_r = min_raise_size(s);
    const double min_to = std::max(max_bet + min_r, max_bet);  // ensure strictly above
    // Max raise-over-call is pot after call, capped by remaining stack
    const double over_call_cap = std::min(pot_after_call(s, p), std::max(0.0, my_stack - atc));
    const double max_to = max_bet + over_call_cap;
    if (max_to >= min_to + 1e-9) {
      out.raise_bounds = RaiseBounds{min_to, max_to};
    }
  }

  return out;
}

std::string to_json(const LegalActionSummary& la) {
  std::ostringstream oss;
  oss << "{";
  oss << "\"can_check\":" << (la.can_check ? "true" : "false") << ",";
  oss << "\"can_fold\":" << (la.can_fold ? "true" : "false") << ",";
  oss << "\"call_amount\":" << la.call_amount << ",";
  if (la.bet_bounds) {
    oss << "\"bet\":{\"min_to\":" << la.bet_bounds->min_to
        << ",\"max_to\":" << la.bet_bounds->max_to << "},";
  }
  if (la.raise_bounds) {
    oss << "\"raise\":{\"min_to\":" << la.raise_bounds->min_to
        << ",\"max_to\":" << la.raise_bounds->max_to << "},";
  }
  oss << "\"suggestions\":[";
  for (size_t i = 0; i < la.suggestions.size(); ++i) {
    const auto& a = la.suggestions[i];
    oss << "{\"type\":" << static_cast<int>(a.type)
        << ",\"amount\":" << a.amount << "}";
    if (i + 1 < la.suggestions.size()) oss << ",";
  }
  oss << "]}";
  return oss.str();
}

}  // namespace quasar
