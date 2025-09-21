#pragma once
#include <string>
#include <vector>

namespace quasar {

struct EquityMatrix {
  int K = 0;
  std::vector<float> data;  // row-major, size K*K
  float at(int i, int j) const { return data[i * K + j]; }
};

// Simple CSV loader: lines of K comma- or space-separated floats; square matrix.
bool load_equity_csv(const std::string& path, EquityMatrix& out);

}  // namespace quasar

