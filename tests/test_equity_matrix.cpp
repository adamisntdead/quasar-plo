#include "quasar/solver/equity_matrix.h"
#include <cassert>
#include <iostream>

int main() {
  quasar::EquityMatrix M;
  bool ok = quasar::load_equity_csv("scripts/goldens/equity_K3.csv", M);
  assert(ok);
  assert(M.K == 3);
  assert(M.at(0,0) == 1.0f);
  assert(M.at(0,1) == 0.6f);
  assert(M.at(2,1) == 0.5f);
  std::cout << "Equity matrix loader tests passed" << std::endl;
  return 0;
}

