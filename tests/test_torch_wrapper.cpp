#include "quasar/nn/value_net.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

int main() {
  const char* path = std::getenv("QUASAR_TS_MODEL");
  if (!path) {
    std::cout << "Skipping TorchScript wrapper test (no QUASAR_TS_MODEL)" << std::endl;
    return 0;
  }
  quasar::IValueNet* net = quasar::load_torchscript_value_net(path);
  if (!net) {
    std::cout << "TorchScript wrapper unavailable or failed to load; skipping" << std::endl;
    return 0;
  }
  const int in = net->input_size();
  const int out = net->output_size();
  assert(in > 0 && out > 0);
  const int B = 2, P = 2;
  std::vector<float> x(B * P * in, 0.f);
  auto y = net->compute_values(x, B, P);
  assert((int)y.size() == B * P * out);
  delete net;
  std::cout << "TorchScript wrapper test passed" << std::endl;
  return 0;
}

