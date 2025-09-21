#pragma once
#include <array>
#include <cstdint>

namespace quasar {

// Utility to extract rank/suit from 0..51 index (2..A ranks -> 0..12)
inline int card_rank(int c) { return c % 13; }
inline int card_suit(int c) { return c / 13; }

// Evaluate a 5-card hand strength key (higher is better).
// Cards are 0..51 using rank/suit mapping above.
uint64_t eval_5card(const std::array<int, 5>& cards);

// Best 5-card hand key for PLO river: exactly 2 from 4 hole + 3 from 5 board.
uint64_t best_plo_river_key(const std::array<int, 4>& hole, const std::array<int, 5>& board);

// Compare two PLO hands on the river. Returns: -1 if A<B, 0 if tie, 1 if A>B.
int compare_plo_river(const std::array<int, 4>& holeA,
                      const std::array<int, 4>& holeB,
                      const std::array<int, 5>& board);

}  // namespace quasar

