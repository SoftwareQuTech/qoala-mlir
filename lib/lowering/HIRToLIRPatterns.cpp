#include "mlir/Transforms/DialectConversion.h"
#include "lowering/HIRToLIRPatterns.h"
#include "Dialect/hir/Hir.h"
#include "Dialect/lir/Lir.h"


mlir::LogicalResult qoala::lowering::NewQubitOpLowering::matchAndRewrite(
        mlir::Operation *op, mlir::ArrayRef<Value> operands,
        mlir::ConversionPatternRewriter &rewriter) const {
    auto loc = op->getLoc();

    auto lirNewQubit = rewriter.create<mlir::lir::NewQubitOp>(loc);
    rewriter.replaceOp(op, lirNewQubit);
    return success();
}