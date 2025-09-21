#pragma once
#include <string>

#include "quasar/engine/public_state.h"
#include "quasar/engine/plo_legal.h"

namespace quasar {

// Parse a minimal JSON spot description into PublicState (best-effort).
bool parse_public_state_from_json(const std::string& json, PublicState& s);

// Assemble the CLI-style JSON response with legal summary and a uniform
// distribution over the provided discrete actions plus check/fold/call as
// applicable.
std::string assemble_response_json(const LegalActionSummary& la,
                                   const std::vector<Action>& discrete);

}  // namespace quasar

