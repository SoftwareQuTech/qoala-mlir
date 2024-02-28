#include "mlir/IR/Diagnostics.h"

#include "Dialect/hir/Passes.h"

namespace mlir {
#define GEN_PASS_DEF_HIRCHECKLINEAR
#include "Dialect/hir/Passes.h.inc"
} // namespace mlir


using namespace mlir;
using namespace mlir::hir;

namespace {

struct HirCheckLinearPass
    : public impl::HirCheckLinearBase<HirCheckLinearPass> {
  void runOnOperation() override;
};

} // namespace

void HirCheckLinearPass::runOnOperation() {
//   RewritePatternSet patterns(&getContext());
//   populateLinalgNamedOpsGeneralizationPatterns(patterns);
//   (void)applyPatternsAndFoldGreedily(getOperation(), std::move(patterns));
    auto operation = getOperation();
    operation->emitError("Hello");

}

std::unique_ptr<Pass> mlir::createHirCheckLinearPass() {
  return std::make_unique<HirCheckLinearPass>();
}
