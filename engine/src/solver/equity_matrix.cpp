#include "quasar/solver/equity_matrix.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace quasar {

static void split_numbers(const std::string& line, std::vector<float>& out) {
  std::stringstream ss(line);
  std::string tok;
  while (std::getline(ss, tok, ',')) {
    // Also split by spaces inside token
    std::stringstream ts(tok);
    std::string sub;
    while (ts >> sub) {
      try {
        out.push_back(std::stof(sub));
      } catch (...) {
      }
    }
  }
}

bool load_equity_csv(const std::string& path, EquityMatrix& out) {
  std::ifstream ifs(path);
  if (!ifs.good()) return false;
  std::vector<std::vector<float>> rows;
  std::string line;
  while (std::getline(ifs, line)) {
    // skip comments and empty lines
    bool only_ws = true; for (char c : line) { if (!std::isspace(static_cast<unsigned char>(c))) { only_ws=false; break; } }
    if (only_ws) continue;
    if (!line.empty() && line[0] == '#') continue;
    std::vector<float> nums;
    split_numbers(line, nums);
    if (!nums.empty()) rows.push_back(std::move(nums));
  }
  if (rows.empty()) return false;
  size_t K = rows[0].size();
  for (const auto& r : rows) if (r.size() != K) return false;
  if (rows.size() != K) return false;
  out.K = static_cast<int>(K);
  out.data.resize(K * K);
  for (size_t i = 0; i < K; ++i) {
    for (size_t j = 0; j < K; ++j) out.data[i * K + j] = rows[i][j];
  }
  return true;
}

}  // namespace quasar

