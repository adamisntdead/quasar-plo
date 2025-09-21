#include "quasar/eval/river.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

namespace quasar {

static inline uint64_t make_key(int cat, const std::array<int, 5>& ranks_desc) {
  // cat in [0..8], ranks 0..12 with 12=Ace high
  uint64_t key = static_cast<uint64_t>(cat & 0xF) << 60;
  for (int i = 0; i < 5; ++i) {
    key |= static_cast<uint64_t>(ranks_desc[i] & 0xF) << (i * 12);
  }
  return key;
}

static inline int straight_high_from_mask(int mask) {
  // mask has bits 0..12 for ranks 2..A
  // Wheel: A-2-3-4-5
  const int wheel = (1 << 12) | (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
  if ((mask & wheel) == wheel) return 3;  // 5-high straight (rank index 3)
  for (int hi = 12; hi >= 4; --hi) {
    int need = (1 << hi) | (1 << (hi - 1)) | (1 << (hi - 2)) | (1 << (hi - 3)) | (1 << (hi - 4));
    if ((mask & need) == need) return hi;
  }
  return -1;
}

uint64_t eval_5card(const std::array<int, 5>& cards) {
  int rank_counts[13] = {0};
  int suit_counts[4] = {0};
  int ranks[5];
  int suits[5];
  int mask = 0;
  for (int i = 0; i < 5; ++i) {
    ranks[i] = card_rank(cards[i]);
    suits[i] = card_suit(cards[i]);
    ++rank_counts[ranks[i]];
    ++suit_counts[suits[i]];
    mask |= (1 << ranks[i]);
  }
  bool is_flush = false;
  for (int s = 0; s < 4; ++s) if (suit_counts[s] == 5) { is_flush = true; break; }
  int straight_hi = straight_high_from_mask(mask);
  bool is_straight = (straight_hi >= 0);

  // Collect ranks sorted descending for high-card based categories
  std::vector<int> rank_list;
  rank_list.reserve(5);
  for (int r = 12; r >= 0; --r) {
    for (int c = 0; c < rank_counts[r]; ++c) rank_list.push_back(r);
  }

  // Analyze groups
  int four = -1, three = -1;
  std::vector<int> pairs;
  for (int r = 12; r >= 0; --r) {
    if (rank_counts[r] == 4) four = r;
    else if (rank_counts[r] == 3) three = r;
    else if (rank_counts[r] == 2) pairs.push_back(r);
  }

  if (is_straight && is_flush) {
    std::array<int,5> ranks_desc{straight_hi, straight_hi-1, straight_hi-2, straight_hi-3, straight_hi-4};
    // wheel special case: represent as 5,4,3,2,A(12)
    if (straight_hi == 3) ranks_desc = {3,2,1,0,12};
    return make_key(8, ranks_desc);
  }
  if (four >= 0) {
    int kicker = -1;
    for (int r : rank_list) if (r != four) { kicker = r; break; }
    std::array<int,5> ranks_desc{four,four,four,four,kicker};
    return make_key(7, ranks_desc);
  }
  if (three >= 0 && !pairs.empty()) {
    std::array<int,5> ranks_desc{three,three,three,pairs[0],pairs[0]};
    return make_key(6, ranks_desc);
  }
  if (is_flush) {
    std::array<int,5> top5{rank_list[0],rank_list[1],rank_list[2],rank_list[3],rank_list[4]};
    return make_key(5, top5);
  }
  if (is_straight) {
    std::array<int,5> ranks_desc{straight_hi, straight_hi-1, straight_hi-2, straight_hi-3, straight_hi-4};
    if (straight_hi == 3) ranks_desc = {3,2,1,0,12};
    return make_key(4, ranks_desc);
  }
  if (three >= 0) {
    // Find two best kickers
    int k1=-1,k2=-1;
    for (int r : rank_list) if (r != three) { if (k1<0) k1=r; else {k2=r; break;} }
    std::array<int,5> ranks_desc{three,three,three,k1,k2};
    return make_key(3, ranks_desc);
  }
  if (pairs.size() >= 2) {
    int p1 = pairs[0];
    int p2 = pairs[1];
    int kicker=-1;
    for (int r : rank_list) if (r != p1 && r != p2) { kicker=r; break; }
    std::array<int,5> ranks_desc{p1,p1,p2,p2,kicker};
    return make_key(2, ranks_desc);
  }
  if (pairs.size() == 1) {
    int p = pairs[0];
    int k1=-1,k2=-1,k3=-1;
    for (int r : rank_list) if (r != p) {
      if (k1<0) k1=r; else if (k2<0) k2=r; else {k3=r; break;}
    }
    std::array<int,5> ranks_desc{p,p,k1,k2,k3};
    return make_key(1, ranks_desc);
  }
  // High card
  std::array<int,5> top5{rank_list[0],rank_list[1],rank_list[2],rank_list[3],rank_list[4]};
  return make_key(0, top5);
}

uint64_t best_plo_river_key(const std::array<int, 4>& hole, const std::array<int, 5>& board) {
  // Enumerate all 2-of-4 hole combos and 3-of-5 board combos
  uint64_t best = 0;
  for (int i = 0; i < 4; ++i) for (int j = i+1; j < 4; ++j) {
    for (int a = 0; a < 5; ++a) for (int b = a+1; b < 5; ++b) for (int c = b+1; c < 5; ++c) {
      std::array<int,5> cards{hole[i], hole[j], board[a], board[b], board[c]};
      uint64_t key = eval_5card(cards);
      if (key > best) best = key;
    }
  }
  return best;
}

int compare_plo_river(const std::array<int, 4>& holeA,
                      const std::array<int, 4>& holeB,
                      const std::array<int, 5>& board) {
  uint64_t ka = best_plo_river_key(holeA, board);
  uint64_t kb = best_plo_river_key(holeB, board);
  if (ka > kb) return 1;
  if (ka < kb) return -1;
  return 0;
}

}  // namespace quasar

