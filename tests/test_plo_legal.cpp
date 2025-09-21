#include "quasar/engine/plo_legal.h"
#include <cassert>
#include <cmath>
#include <iostream>

using quasar::ActionType;
using quasar::LegalActionSummary;
using quasar::PublicState;

static inline bool approx(double a, double b, double eps = 1e-6) {
  return std::fabs(a - b) < eps;
}

int main() {
  {
    // Preflop: SB=1, BB=2. SB to act facing 1.
    PublicState s;
    s.num_players = 2;
    s.sb = 1.0; s.bb = 2.0; s.ante = 0.0;
    s.street = 0;
    s.button = 1;  // button is BB in heads-up
    s.player_to_act = 0; // SB
    s.stacks = {98.0, 100.0};
    s.committed_on_street = {1.0, 2.0};
    s.committed_total = {1.0, 2.0};
    s.last_raise_size = 0.0;  // no voluntary raise yet

    auto la = quasar::compute_legal_actions(s);
    // SB can call 1
    assert(approx(la.call_amount, 1.0));
    // Min raise to should be 4 (bb min-raise over 2)
    assert(la.raise_bounds.has_value());
    assert(approx(la.raise_bounds->min_to, 4.0));
    // Pot raise target should be 6
    bool found_pot = false;
    for (auto& a : la.suggestions) {
      if (a.type == ActionType::kRaise && approx(a.amount, 6.0)) {
        found_pot = true;
      }
    }
    assert(found_pot);
  }

  {
    // Flop: pot=10, no bet. Min bet = bb, max bet = pot
    PublicState s;
    s.num_players = 2;
    s.sb = 1.0; s.bb = 2.0; s.ante = 0.0;
    s.street = 1;
    s.player_to_act = 0;
    s.stacks = {100.0, 100.0};
    s.committed_on_street = {0.0, 0.0};
    s.committed_total = {5.0, 5.0};  // pot=10
    s.last_raise_size = 0.0;

    auto la = quasar::compute_legal_actions(s);
    assert(la.bet_bounds.has_value());
    assert(approx(la.bet_bounds->min_to, 2.0));
    assert(approx(la.bet_bounds->max_to, 10.0));
  }

  std::cout << "PLO legality tests passed" << std::endl;
  return 0;
}

