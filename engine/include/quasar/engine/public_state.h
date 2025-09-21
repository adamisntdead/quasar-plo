#pragma once
#include <string>
#include <vector>

namespace quasar {

// PublicState captures minimal information needed for action legality.
// This is NOT a full PLO state; it omits private cards and side pots.
struct PublicState {
  // Players
  int num_players = 2;
  int player_to_act = 0;            // index of player to act
  int button = 0;                   // dealer/button index

  // Streets: 0=preflop,1=flop,2=turn,3=river
  int street = 0;
  std::vector<int> board;           // up to 5 card indices (0..51), pad with -1

  // Betting parameters
  double sb = 1.0;
  double bb = 2.0;
  double ante = 0.0;                // equal ante for all players

  // Chip accounting (chip units are arbitrary but consistent across fields)
  // - stacks[i]: remaining stack behind for player i
  // - committed_total[i]: total committed across the hand (all streets)
  // - committed_on_street[i]: committed during current betting round
  std::vector<double> stacks;
  std::vector<double> committed_total;
  std::vector<double> committed_on_street;

  // Last aggressive action size on this street (raise size, not total-to)
  // If 0 and facing a live bet (preflop blinds), min-raise size defaults to bb.
  double last_raise_size = 0.0;

  // Derived helpers
  // Sum of committed_total across players + antes already posted.
  double pot_total() const;
  double current_bet_to_call() const;  // maximum committed_on_street across players
  double contributed_this_street(int p) const;
  double amount_to_call(int p) const;

  std::string to_string() const;
};

}  // namespace quasar
