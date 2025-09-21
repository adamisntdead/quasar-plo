#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "quasar/engine/plo_legal.h"
#include "quasar/engine/json.h"
#include "quasar/engine/discretize.h"

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
  quasar::parse_public_state_from_json(input, s);
  quasar::BettingRules rules;
  quasar::parse_rules_from_json(input, rules);
  auto la = quasar::compute_legal_actions(s, rules);
  quasar::DiscretizationConfig cfg = quasar::parse_discretization_from_json(input);
  quasar::SolveOneConfig so_cfg;
  so_cfg.rules = rules;
  so_cfg.discretization = cfg;
  // Optional solver config: {"solver": {"type":"cfr", "iters":N}}
  int cfr_iters = 0;
  auto spos = input.find("\"solver\"");
  if (spos != std::string::npos) {
    auto tpos = input.find("\"iters\"", spos);
    if (tpos != std::string::npos) {
      // crude parse of number after iters:
      auto colon = input.find(':', tpos);
      if (colon != std::string::npos) cfr_iters = static_cast<int>(std::strtod(input.c_str() + colon + 1, nullptr));
    }
  }
  so_cfg.cfr_iters = cfr_iters;
  auto res = quasar::solve_one(s, so_cfg);
  std::cout << quasar::assemble_response_json(res.legal, res.actions, res.probabilities) << std::endl;
  return 0;
}
