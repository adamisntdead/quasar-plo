#pragma once
#include <cstdint>

namespace quasar {

enum class Street : int32_t { kPreflop = 0, kFlop = 1, kTurn = 2, kRiver = 3 };

enum class ActionType : int32_t {
  kFold = 0,
  kCheck = 1,
  kCall = 2,
  kBet = 3,
  kRaise = 4,
  kAllIn = 5,
};

}  // namespace quasar

