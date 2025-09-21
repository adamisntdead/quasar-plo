#pragma once
#include <string>

#include "quasar/engine/public_state.h"
#include "quasar/engine/plo_legal.h"
#include "quasar/engine/rules.h"
#include "quasar/engine/discretize.h"

namespace quasar {

// Parse a minimal JSON spot description into PublicState (best-effort).
bool parse_public_state_from_json(const std::string& json, PublicState& s);

// Optional config parsing
bool parse_rules_from_json(const std::string& json, BettingRules& rules);
DiscretizationConfig parse_discretization_from_json(const std::string& json,
                                                    const DiscretizationConfig& def = DiscretizationConfig{});

// Assemble the CLI-style JSON response with legal summary and a uniform
// distribution over the provided discrete actions plus check/fold/call as
// applicable.
// Assemble response with uniform distribution over [check/fold/call]+discrete
std::string assemble_response_json(const LegalActionSummary& la,
                                   const std::vector<Action>& discrete);

// Assemble response with explicit actions and probabilities.
std::string assemble_response_json(const LegalActionSummary& la,
                                   const std::vector<Action>& actions,
                                   const std::vector<double>& probs);

}  // namespace quasar
