#include "quasar/engine/public_state.h"
#include <sstream>

namespace quasar {

std::string PublicState::to_string() const {
  std::ostringstream oss;
  oss << "to_act=" << player_to_act
      << ", street=" << street
      << ", pot=" << pot
      << ", board=[";
  for (size_t i = 0; i < board.size(); ++i) {
    oss << board[i] << (i + 1 < board.size() ? "," : "]");
  }
  oss << ", stacks=[";
  for (size_t i = 0; i < stacks.size(); ++i) {
    oss << stacks[i] << (i + 1 < stacks.size() ? "," : "]");
  }
  return oss.str();
}

}  // namespace quasar

