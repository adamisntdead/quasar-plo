#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>

#include "quasar/engine/json.h"
#include "quasar/engine/solve_one.h"

using namespace std::chrono;

static std::string slurp(const std::string& path) {
  std::ifstream ifs(path);
  std::stringstream buf; buf << ifs.rdbuf();
  return buf.str();
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "usage: bench_solve_one <spot.json> [iters]" << std::endl;
    return 2;
  }
  const std::string json = slurp(argv[1]);
  int iters = (argc > 2 ? std::atoi(argv[2]) : 10000);
  quasar::PublicState s;
  quasar::parse_public_state_from_json(json, s);
  quasar::BettingRules rules; quasar::parse_rules_from_json(json, rules);
  quasar::SolveOneConfig cfg; cfg.rules = rules; cfg.discretization = quasar::parse_discretization_from_json(json);

  // Warmup
  for (int i = 0; i < 100; ++i) (void)quasar::solve_one(s, cfg);

  auto t0 = high_resolution_clock::now();
  for (int i = 0; i < iters; ++i) (void)quasar::solve_one(s, cfg);
  auto t1 = high_resolution_clock::now();
  auto us = duration_cast<microseconds>(t1 - t0).count();
  double per_call_us = static_cast<double>(us) / iters;
  std::cout << "solve_one latency: " << per_call_us << " us (avg over " << iters << ")" << std::endl;
  return 0;
}

