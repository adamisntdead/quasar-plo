#include "quasar/nn/value_net.h"

#ifdef QUASAR_USE_TORCH
#include <torch/script.h>
#endif

#include <memory>

namespace quasar {

#ifdef QUASAR_USE_TORCH
class TorchscriptValueNet final : public IValueNet {
 public:
  explicit TorchscriptValueNet(const std::string& path) {
    module_ = torch::jit::load(path);
    // Read attributes if present
    input_size_ = get_attr<int64_t>("input_size", 0);
    output_size_ = get_attr<int64_t>("output_size", 0);
    is_sparse_ = get_attr<bool>("is_sparse", false);
  }

  int input_size() const override { return static_cast<int>(input_size_); }
  int output_size() const override { return static_cast<int>(output_size_); }
  bool is_sparse() const override { return is_sparse_; }

  std::vector<float> compute_values(const std::vector<float>& queries, int batch, int players) override {
    const int in_size = input_size();
    const int out_size = output_size();
    std::vector<int64_t> shape = {batch, players, in_size};
    auto options = torch::TensorOptions().dtype(torch::kFloat32);
    torch::Tensor x = torch::from_blob(const_cast<float*>(queries.data()), shape, options).clone();
    torch::Tensor mask = torch::ones({batch, players}, options);
    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(x);
    inputs.push_back(mask);
    auto out = module_.forward(inputs).toTensor();
    auto out_contig = out.contiguous();
    std::vector<float> result(out_contig.numel());
    std::memcpy(result.data(), out_contig.data_ptr<float>(), result.size() * sizeof(float));
    return result;
  }

 private:
  template <typename T>
  T get_attr(const char* name, T def) {
    if (module_.has_attribute(name)) {
      return module_.attr(name).to<T>();
    }
    return def;
  }

  torch::jit::script::Module module_;
  int64_t input_size_ = 0;
  int64_t output_size_ = 0;
  bool is_sparse_ = false;
};
#endif

IValueNet* load_torchscript_value_net(const std::string& path) {
#ifdef QUASAR_USE_TORCH
  try {
    return new TorchscriptValueNet(path);
  } catch (...) {
    return nullptr;
  }
#else
  (void)path;
  return nullptr;
#endif
}

}  // namespace quasar

