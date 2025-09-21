#include "quasar/eval/river.h"
#include <array>
#include <cassert>
#include <iostream>

int main() {
  // Construct a board with three clubs (suit 0) to allow flush
  // Board clubs ranks: 2,6,10 => indices 0,4,8; add two offsuit cards 13 (suit1,2) and 26 (suit2,2)
  std::array<int,5> board{0,4,8,13,26};
  // Hand A: clubs K (11), A (12) plus two off-suit
  std::array<int,4> handA{11,12,14,27};
  // Hand B: clubs 7 (5?), 9 (7?) use indices 7 and 9 for clubs; plus off-suit
  std::array<int,4> handB{7,9,15,28};

  int cmp = quasar::compare_plo_river(handA, handB, board);
  assert(cmp == 1);  // A-high flush beats lower flush

  // Straight vs. trips scenario
  // Board: 5,6,7, K, 2 of mixed suits -> indices 3,4,5,11,0
  std::array<int,5> board2{3,4,5,11,0};
  // Hand C: 8 and 9 (ranks 6,7) to make straight 5-9 with 3-of-board
  std::array<int,4> handC{6,7,20,21};
  // Hand D: pair of Kings (rank 11) and junk -> would make trips K but should lose to straight
  std::array<int,4> handD{11+13, 11+26, 30, 31};
  int cmp2 = quasar::compare_plo_river(handC, handD, board2);
  assert(cmp2 == 1);

  std::cout << "PLO river evaluator tests passed" << std::endl;
  return 0;
}

