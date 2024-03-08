#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

#include "Dialect/hir/Passes.h"

namespace mlir::hir {
#define GEN_PASS_DEF_HIRCHECKLINEAR
#include "Dialect/hir/Passes.h.inc"

namespace {
class HirCheckLinearRewriter : public OpRewritePattern<func::FuncOp> {
public:
  using OpRewritePattern<func::FuncOp>::OpRewritePattern;
  LogicalResult matchAndRewrite(func::FuncOp op,
                                PatternRewriter &rewriter) const final {
    // TODO - Re-implement this
    if (op.getSymName() == "bar") {
      rewriter.updateRootInPlace(op, [&op]() { op.setSymName("foo"); });
      return success();
    }
    return failure();
  }
};

class HirCheckLinear
    : public impl::HirCheckLinearBase<HirCheckLinear> {
public:
  using impl::HirCheckLinearBase<
          HirCheckLinear>::HirCheckLinearBase;
  void runOnOperation() final {
    RewritePatternSet patterns(&getContext());
    patterns.add<HirCheckLinearRewriter>(&getContext());
    FrozenRewritePatternSet patternSet(std::move(patterns));
    if (failed(applyPatternsAndFoldGreedily(getOperation(), patternSet)))
      signalPassFailure();
  }
};
} // namespace
} // namespace mlir::hir
