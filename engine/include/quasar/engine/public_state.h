#pragma once
#include <string>
#include <vector>

namespace quasar {

// Minimal public state stub. Extend with PLO-specific fields.
struct PublicState {
  int player_to_act = 0;            // 0 or 1
  int street = 0;                   // 0=pre,1=flop,2=turn,3=river
  std::vector<int> board;           // up to 5 card indices (0..51)
  double pot = 0.0;                 // total pot
  std::vector<double> stacks;       // per-player stacks

  std::string to_string() const;
};

}  // namespace quasar

