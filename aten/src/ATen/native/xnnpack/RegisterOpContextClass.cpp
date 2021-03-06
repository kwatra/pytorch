#ifdef USE_XNNPACK

#include <ATen/core/op_registration/op_registration.h>
#include <ATen/native/xnnpack/Convolution.h>
#include <ATen/native/xnnpack/Linear.h>
#include <ATen/native/xnnpack/OpContext.h>
#include <ATen/Tensor.h>
#include <torch/custom_class.h>

namespace at {
namespace native {
namespace xnnpack {

using internal::linear::createLinearClampPrePackOpContext;
using internal::convolution2d::createConv2dClampPrePackOpContext;

namespace {
torch::jit::class_<LinearOpContext> register_packed_linear_op_context_class() {
  static auto register_linear_op_context_class =
      torch::jit::class_<LinearOpContext>("xnnpack", "LinearOpContext")
          .def_pickle(
              [](const c10::intrusive_ptr<LinearOpContext>& op_context)
                  -> SerializationTypeLinearPrePack { // __getstate__
                return op_context->unpack();
              },
              [](SerializationTypeLinearPrePack state)
                  -> c10::intrusive_ptr<LinearOpContext> { // __setstate__
                return createLinearClampPrePackOpContext(
                    std::move(std::get<0>(state)),
                    std::move(std::get<1>(state)),
                    std::move(std::get<2>(state)),
                    std::move(std::get<3>(state)));
              });
  return register_linear_op_context_class;
}

torch::jit::class_<Conv2dOpContext> register_packed_conv2d_op_context_class() {
  static auto register_conv2d_op_context_class =
      torch::jit::class_<Conv2dOpContext>("xnnpack", "Conv2dOpContext")
          .def_pickle(
              [](const c10::intrusive_ptr<Conv2dOpContext>& op_context)
                  -> SerializationTypeConv2dPrePack { // __getstate__
                return op_context->unpack();
              },
              [](SerializationTypeConv2dPrePack state)
                  -> c10::intrusive_ptr<Conv2dOpContext> { // __setstate__
                return createConv2dClampPrePackOpContext(
                    std::move(std::get<0>(state)),
                    std::move(std::get<1>(state)),
                    std::move(std::get<2>(state)),
                    std::move(std::get<3>(state)),
                    std::move(std::get<4>(state)),
                    std::move(std::get<5>(state)),
                    std::move(std::get<6>(state)),
                    std::move(std::get<7>(state)));
              });
  return register_conv2d_op_context_class;
}

static auto linear_op_context_class = register_packed_linear_op_context_class();
static auto conv2d_op_context_class = register_packed_conv2d_op_context_class();

// TODO: These should probably be in an xnnpack namespace?
// https://github.com/pytorch/pytorch/issues/36517
TORCH_LIBRARY(prepacked, m) {
  m.def("linear_clamp_prepack(Tensor W, Tensor? B=None, Scalar? output_min=None, Scalar? output_max=None) -> __torch__.torch.classes.xnnpack.LinearOpContext");
  m.def("linear_clamp_run(Tensor X, __torch__.torch.classes.xnnpack.LinearOpContext W_prepack) -> Tensor Y");
  m.def("conv2d_clamp_prepack(Tensor W, Tensor? B, int[2] stride, int[2] padding, int[2] dilation, int groups, Scalar? output_min=None, Scalar? output_max=None) -> __torch__.torch.classes.xnnpack.Conv2dOpContext");
  m.def("conv2d_clamp_run(Tensor X, __torch__.torch.classes.xnnpack.Conv2dOpContext W_prepack) -> Tensor Y");
}

// Op registeration
static auto registry =
    torch::RegisterOperators()
        .op("prepacked::linear_clamp_prepack(Tensor W, Tensor? B=None, "
            "Scalar? output_min=None, Scalar? output_max=None) "
            "-> __torch__.torch.classes.xnnpack.LinearOpContext",
            torch::RegisterOperators::options()
            .aliasAnalysis(at::AliasAnalysisKind::FROM_SCHEMA)
            .kernel<decltype(createLinearClampPrePackOpContext),
                createLinearClampPrePackOpContext>(
                    DispatchKey::CPU))
        .op("prepacked::linear_clamp_run(Tensor X,"
            " __torch__.torch.classes.xnnpack.LinearOpContext W_prepack) -> Tensor Y",
            torch::RegisterOperators::options()
            .aliasAnalysis(at::AliasAnalysisKind::FROM_SCHEMA)
            .kernel<internal::linear::LinearClampRun>(
                DispatchKey::CPU))
        .op("prepacked::conv2d_clamp_prepack(Tensor W, Tensor? B, int[2] stride, "
            "int[2] padding, int[2] dilation, int groups, "
            "Scalar? output_min=None, Scalar? output_max=None) "
            "-> __torch__.torch.classes.xnnpack.Conv2dOpContext",
            torch::RegisterOperators::options()
            .aliasAnalysis(at::AliasAnalysisKind::FROM_SCHEMA)
            .kernel<decltype(createConv2dClampPrePackOpContext),
                createConv2dClampPrePackOpContext>(
                DispatchKey::CPU))
        .op("prepacked::conv2d_clamp_run(Tensor X, "
            "__torch__.torch.classes.xnnpack.Conv2dOpContext W_prepack) -> Tensor Y",
            torch::RegisterOperators::options()
            .aliasAnalysis(at::AliasAnalysisKind::FROM_SCHEMA)
            .kernel<internal::convolution2d::Conv2dClampRun>(
                DispatchKey::CPU));
} // namespace

} // namespace xnnpack
} // namespace native
} // namespace at

#endif /* USE_XNNPACK */
