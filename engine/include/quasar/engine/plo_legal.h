#pragma once
#include <optional>
#include <string>
#include <vector>

#include "quasar/engine/public_state.h"
#include "quasar/engine/types.h"

namespace quasar {

struct Action {
  ActionType type;
  // For kBet/kRaise: target "to" amount (new bet level on this street).
  // For kCall: amount paid this action.
  double amount = 0.0;
};

struct RaiseBounds {
  // Legal raise/bet target range [min_to, max_to], if any
  double min_to = 0.0;
  double max_to = 0.0;
};

struct LegalActionSummary {
  bool can_check = false;
  bool can_fold = false;  // true iff facing action and stack > 0
  double call_amount = 0.0;  // 0 if check is allowed
  std::optional<RaiseBounds> bet_bounds;    // present if no bet yet
  std::optional<RaiseBounds> raise_bounds;  // present if facing a live bet
  // Helpful discrete suggestions (subset of continuum)
  std::vector<Action> suggestions;
};

// Compute pot-limit legal action information for the player_to_act.
// Assumes PublicState fields are well-formed (sizes match num_players).
LegalActionSummary compute_legal_actions(const PublicState& s);

// Utility helpers exported for tests/CLI
double pot_after_call(const PublicState& s, int player);
double min_raise_size(const PublicState& s);

// Small JSON serialization helpers for CLI
std::string to_json(const LegalActionSummary& la);

}  // namespace quasar

