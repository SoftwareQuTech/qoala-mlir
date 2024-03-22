#ifndef QOALA_MLIR_HIRTOLIRPATTERNS_H
#define QOALA_MLIR_HIRTOLIRPATTERNS_H
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/Debug.h"
#include "Dialect/hir/Hir.h"
#include "Dialect/lir/Lir.h"

namespace qoala::lowering {
class HirQubitToLirQubitTypeConverter : public TypeConverter {
public:
    explicit HirQubitToLirQubitTypeConverter(MLIRContext *ctx);
};

template <typename SourceOp, typename DestOp>
class OpLoweringTemplate : public OpConversionPattern<SourceOp> {
public:
    // Constructor simply matches the super class
    using OpConversionPattern<SourceOp>::OpConversionPattern;

    LogicalResult matchAndRewrite(
            SourceOp op, typename SourceOp::Adaptor adaptor,
            ConversionPatternRewriter &rewriter) const override {
        llvm::dbgs() << "lowering operation : '" << op << "'\n";
        auto newOp = rewriter.replaceOpWithNewOp<DestOp>(op, adaptor.getOperands());
        return success();
    }
};

using MeasureQubitOpLowering = OpLoweringTemplate<mlir::hir::MeasureOp, mlir::lir::MeasureOp>;

using NewQubitOpLowering = OpLoweringTemplate<mlir::hir::NewQubitOp, mlir::lir::NewQubitOp>;

using RotZOpLowering = OpLoweringTemplate<mlir::hir::RotZOp, mlir::lir::RotateZOp>;

// TODO - instantiate the template to map operations from one dialect to the other

} //namespace qoala::lowering

#endif //QOALA_MLIR_HIRTOLIRPATTERNS_H
