#pragma once

namespace quasar {

struct BettingRules {
  enum class MinBetRule { BigBlind, OneChip };
  MinBetRule min_bet_rule = MinBetRule::BigBlind;
};

}  // namespace quasar

