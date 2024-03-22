#ifndef QOALA_MLIR_HIRTOLIRPATTERNS_H
#define QOALA_MLIR_HIRTOLIRPATTERNS_H
#include "mlir/Transforms/DialectConversion.h"
#include "Dialect/hir/Hir.h"

namespace qoala::lowering {
class NewQubitOpLowering : public ConversionPattern {
public:
    explicit NewQubitOpLowering(MLIRContext *ctx)
            : ConversionPattern(mlir::hir::NewQubitOp::getOperationName(), 1, ctx) { }
    LogicalResult matchAndRewrite(
            Operation *op, ArrayRef<Value> operands,
            ConversionPatternRewriter &rewriter) const override;
};
} //namespace qoala::lowering

#endif //QOALA_MLIR_HIRTOLIRPATTERNS_H
