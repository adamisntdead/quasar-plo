#include "quasar/engine/json.h"

#include <cstdlib>
#include <sstream>

namespace quasar {

static std::string get_string(const std::string& s, const std::string& key) {
  auto pos = s.find("\"" + key + "\"");
  if (pos == std::string::npos) return "";
  pos = s.find(':', pos);
  if (pos == std::string::npos) return "";
  auto start = s.find('"', pos + 1);
  if (start == std::string::npos) return "";
  auto end = s.find('"', start + 1);
  if (end == std::string::npos) return "";
  return s.substr(start + 1, end - start - 1);
}

static double get_number(const std::string& s, const std::string& key, double def = 0.0) {
  auto pos = s.find("\"" + key + "\"");
  if (pos == std::string::npos) return def;
  pos = s.find(':', pos);
  if (pos == std::string::npos) return def;
  char* end = nullptr;
  const char* start = s.c_str() + pos + 1;
  double val = std::strtod(start, &end);
  if (start == end) return def;
  return val;
}

static std::vector<double> get_number_array(const std::string& s, const std::string& key) {
  std::vector<double> out;
  auto pos = s.find("\"" + key + "\"");
  if (pos == std::string::npos) return out;
  pos = s.find('[', pos);
  if (pos == std::string::npos) return out;
  auto end = s.find(']', pos);
  if (end == std::string::npos) return out;
  std::string arr = s.substr(pos + 1, end - pos - 1);
  std::stringstream ss(arr);
  while (ss.good()) {
    std::string tok;
    std::getline(ss, tok, ',');
    if (tok.empty()) continue;
    out.push_back(std::atof(tok.c_str()));
  }
  return out;
}

static std::vector<int> get_int_array(const std::string& s, const std::string& key) {
  std::vector<int> out;
  auto nums = get_number_array(s, key);
  out.reserve(nums.size());
  for (double d : nums) out.push_back(static_cast<int>(d));
  return out;
}

bool parse_public_state_from_json(const std::string& input, PublicState& s) {
  const std::string street = get_string(input, "street");
  s.street = 0;
  if (street == "flop") s.street = 1; else if (street == "turn") s.street = 2; else if (street == "river") s.street = 3;
  s.sb = get_number(input, "sb", 1.0);
  s.bb = get_number(input, "bb", 2.0);
  s.ante = get_number(input, "ante", 0.0);
  s.player_to_act = static_cast<int>(get_number(input, "to_act", 0));
  s.button = static_cast<int>(get_number(input, "button", 0));
  s.last_raise_size = get_number(input, "last_raise_size", 0.0);
  s.stacks = get_number_array(input, "stacks");
  s.num_players = static_cast<int>(s.stacks.size());
  s.committed_total = get_number_array(input, "committed_total");
  s.committed_on_street = get_number_array(input, "committed_on_street");
  s.board = get_int_array(input, "board");
  return s.num_players >= 2 && s.stacks.size() == s.committed_total.size() && s.stacks.size() == s.committed_on_street.size();
}

bool parse_rules_from_json(const std::string& input, BettingRules& rules) {
  // Optional: "min_bet_rule": "BigBlind" | "OneChip"
  const std::string mbr = get_string(input, "min_bet_rule");
  if (!mbr.empty()) {
    if (mbr == "OneChip") rules.min_bet_rule = BettingRules::MinBetRule::OneChip;
    else rules.min_bet_rule = BettingRules::MinBetRule::BigBlind;
  }
  return true;
}

DiscretizationConfig parse_discretization_from_json(const std::string& input, const DiscretizationConfig& def) {
  DiscretizationConfig cfg = def;
  // Optional: {
  //   "discretization": {
  //      "pot_fracs": [..],
  //      "include_min": true/false,
  //      "include_pot_raise": true/false,
  //      "include_all_in": true/false
  //   }
  // }
  auto pos = input.find("\"discretization\"");
  if (pos != std::string::npos) {
    // pot_fracs
    // Find pot_fracs array relative to discretization block
    auto pf_pos = input.find("\"pot_fracs\"", pos);
    if (pf_pos != std::string::npos) {
      // temporarily slice from pf_pos to extract array
      std::vector<double> pf = get_number_array(input.substr(pf_pos), "pot_fracs");
      if (!pf.empty()) cfg.pot_fracs = pf;
    }
    // Booleans: include_min, include_pot_raise, include_all_in
    auto get_bool = [&](const std::string& key, bool defv) {
      auto kpos = input.find("\"" + key + "\"", pos);
      if (kpos == std::string::npos) return defv;
      kpos = input.find(':', kpos);
      if (kpos == std::string::npos) return defv;
      // look for true/false
      auto tpos = input.find("true", kpos);
      auto fpos = input.find("false", kpos);
      if (tpos != std::string::npos && (fpos == std::string::npos || tpos < fpos)) return true;
      if (fpos != std::string::npos && (tpos == std::string::npos || fpos < tpos)) return false;
      return defv;
    };
    cfg.include_min = get_bool("include_min", cfg.include_min);
    cfg.include_pot_raise = get_bool("include_pot_raise", cfg.include_pot_raise);
    cfg.include_all_in = get_bool("include_all_in", cfg.include_all_in);
  }
  return cfg;
}

std::string assemble_response_json(const LegalActionSummary& la,
                                   const std::vector<Action>& discrete) {
  // Base discrete actions + check/fold/call as appropriate → uniform
  struct UA { int type; double amount; double prob; };
  std::vector<UA> ua;
  if (la.can_check) {
    ua.push_back({static_cast<int>(ActionType::kCheck), 0.0, 0.0});
  } else {
    if (la.can_fold) ua.push_back({static_cast<int>(ActionType::kFold), 0.0, 0.0});
    ua.push_back({static_cast<int>(ActionType::kCall), la.call_amount, 0.0});
  }
  for (auto& a : discrete) ua.push_back({static_cast<int>(a.type), a.amount, 0.0});
  const double n = static_cast<double>(std::max<size_t>(1, ua.size()));
  for (auto& a : ua) a.prob = 1.0 / n;

  std::ostringstream oss;
  oss << "{";
  oss << "\"legal\":" << to_json(la) << ",";
  oss << "\"uniform_actions\":[";
  for (size_t i = 0; i < ua.size(); ++i) {
    oss << "{\"type\":" << ua[i].type << ",\"amount\":" << ua[i].amount
        << ",\"prob\":" << ua[i].prob << "}";
    if (i + 1 < ua.size()) oss << ",";
  }
  oss << "]}";
  return oss.str();
}

std::string assemble_response_json(const LegalActionSummary& la,
                                   const std::vector<Action>& actions,
                                   const std::vector<double>& probs) {
  // Emit JSON with provided probabilities
  std::ostringstream oss;
  oss << "{";
  oss << "\"legal\":" << to_json(la) << ",";
  oss << "\"uniform_actions\":[";  // keep key name for backward compat
  for (size_t i = 0; i < actions.size(); ++i) {
    const auto& a = actions[i];
    const double p = (i < probs.size() ? probs[i] : 0.0);
    oss << "{\"type\":" << static_cast<int>(a.type) << ",\"amount\":" << a.amount
        << ",\"prob\":" << p << "}";
    if (i + 1 < actions.size()) oss << ",";
  }
  oss << "]}";
  return oss.str();
}

}  // namespace quasar
