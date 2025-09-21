#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "quasar/engine/plo_legal.h"

// Minimal JSON reader for a known schema using a tiny hand-rolled parser.
// Expected keys:
// - street: "preflop"|"flop"|"turn"|"river"
// - sb, bb, ante: numbers
// - to_act, button: integers
// - stacks: [..]
// - committed_total: [..]
// - committed_on_street: [..]
// - last_raise_size: number (optional)
// - board: [..] (optional)

static std::string slurp(const std::string& path) {
  std::ifstream ifs(path);
  std::stringstream buffer;
  buffer << ifs.rdbuf();
  return buffer.str();
}

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

static int get_int(const std::string& s, const std::string& key, int def = 0) {
  return static_cast<int>(get_number(s, key, def));
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

int main(int argc, char** argv) {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  std::string input;
  if (argc > 1) {
    input = slurp(argv[1]);
  } else {
    std::stringstream buf;
    buf << std::cin.rdbuf();
    input = buf.str();
  }

  quasar::PublicState s;
  const std::string street = get_string(input, "street");
  if (street == "preflop") s.street = 0; else if (street == "flop") s.street = 1; else if (street == "turn") s.street = 2; else if (street == "river") s.street = 3;
  s.sb = get_number(input, "sb", 1.0);
  s.bb = get_number(input, "bb", 2.0);
  s.ante = get_number(input, "ante", 0.0);
  s.player_to_act = get_int(input, "to_act", 0);
  s.button = get_int(input, "button", 0);
  s.last_raise_size = get_number(input, "last_raise_size", 0.0);
  s.stacks = get_number_array(input, "stacks");
  s.num_players = static_cast<int>(s.stacks.size());
  s.committed_total = get_number_array(input, "committed_total");
  s.committed_on_street = get_number_array(input, "committed_on_street");
  s.board = get_int_array(input, "board");

  auto la = quasar::compute_legal_actions(s);
  // Build a discrete action set and uniform probabilities
  struct UA { int type; double amount; double prob; };
  std::vector<UA> ua;
  if (la.can_check) {
    ua.push_back({static_cast<int>(quasar::ActionType::kCheck), 0.0, 0.0});
  } else {
    if (la.can_fold) ua.push_back({static_cast<int>(quasar::ActionType::kFold), 0.0, 0.0});
    ua.push_back({static_cast<int>(quasar::ActionType::kCall), la.call_amount, 0.0});
  }
  for (auto& a : la.suggestions) {
    ua.push_back({static_cast<int>(a.type), a.amount, 0.0});
  }
  const double n = static_cast<double>(std::max<size_t>(1, ua.size()));
  for (auto& a : ua) a.prob = 1.0 / n;

  // Emit JSON
  std::ostringstream oss;
  oss << "{";
  oss << "\"legal\":" << quasar::to_json(la) << ",";
  oss << "\"uniform_actions\":[";
  for (size_t i = 0; i < ua.size(); ++i) {
    oss << "{\"type\":" << ua[i].type << ",\"amount\":" << ua[i].amount
        << ",\"prob\":" << ua[i].prob << "}";
    if (i + 1 < ua.size()) oss << ",";
  }
  oss << "]}";

  std::cout << oss.str() << std::endl;
  return 0;
}
