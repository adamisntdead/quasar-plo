#include "quasar/engine/public_state.h"
#include <algorithm>
#include <numeric>
#include <sstream>

namespace quasar {

std::string PublicState::to_string() const {
  std::ostringstream oss;
  oss << "players=" << num_players
      << ", btn=" << button
      << ", to_act=" << player_to_act
      << ", street=" << street
      << ", sb/bb=" << sb << "/" << bb
      << ", board=[";
  for (size_t i = 0; i < board.size(); ++i) {
    oss << board[i] << (i + 1 < board.size() ? "," : "]");
  }
  oss << ", stacks=[";
  for (size_t i = 0; i < stacks.size(); ++i) {
    oss << stacks[i] << (i + 1 < stacks.size() ? "," : "]");
  }
  oss << ", committed_total=[";
  for (size_t i = 0; i < committed_total.size(); ++i) {
    oss << committed_total[i] << (i + 1 < committed_total.size() ? "," : "]");
  }
  oss << ", on_street=[";
  for (size_t i = 0; i < committed_on_street.size(); ++i) {
    oss << committed_on_street[i] << (i + 1 < committed_on_street.size() ? "," : "]");
  }
  oss << ", last_raise_sz=" << last_raise_size;
  return oss.str();
}

double PublicState::pot_total() const {
  // Pot is sum of committed_total across players (antes are assumed included
  // into committed_total by the state builder). This includes uncalled bets.
  return std::accumulate(committed_total.begin(), committed_total.end(), 0.0);
}

double PublicState::current_bet_to_call() const {
  if (committed_on_street.empty()) return 0.0;
  return *std::max_element(committed_on_street.begin(), committed_on_street.end());
}

double PublicState::contributed_this_street(int p) const {
  if (p < 0 || p >= static_cast<int>(committed_on_street.size())) return 0.0;
  return committed_on_street[p];
}

double PublicState::amount_to_call(int p) const {
  double max_bet = current_bet_to_call();
  double me = contributed_this_street(p);
  return std::max(0.0, max_bet - me);
}

}  // namespace quasar
