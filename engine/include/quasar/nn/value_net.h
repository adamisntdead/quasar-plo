#pragma once
#include <string>
#include <vector>

namespace quasar {

class IValueNet {
 public:
  virtual ~IValueNet() = default;
  virtual int input_size() const = 0;
  virtual int output_size() const = 0;
  virtual bool is_sparse() const = 0;
  // queries: [batch, players, input_size]
  // returns: [batch, players, output_size]
  virtual std::vector<float> compute_values(const std::vector<float>& queries,
                                            int batch,
                                            int players) = 0;
};

// Factory: returns nullptr if TorchScript is not enabled/available.
IValueNet* load_torchscript_value_net(const std::string& path);

}  // namespace quasar

