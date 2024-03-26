#ifndef QNET_TO_QMEM_PATTERNS : _H
#define QNET_TO_QMEM_PATTERNS:_H
#include "Dialect/Qnet/Qnet.h"
#include "Dialect/lir/Lir.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/Debug.h"

using namespace qoala::dialects;

namespace qoala::conversion {
class QnetToQmemQubitTypeConverter : public TypeConverter {
  public:
    explicit QnetToQmemQubitTypeConverter(MLIRContext *ctx);
};

template <typename SourceOp, typename DestOp>
class OpLoweringTemplate : public OpConversionPattern<SourceOp> {
  public:
    // Constructor simply matches the super class
    using OpConversionPattern<SourceOp>::OpConversionPattern;

    LogicalResult
    matchAndRewrite(SourceOp op, typename SourceOp::Adaptor adaptor,
                    ConversionPatternRewriter &rewriter) const override {
        llvm::dbgs() << "lowering operation : '" << op << "'\n";
        auto newOp =
            rewriter.replaceOpWithNewOp<DestOp>(op, adaptor.getOperands());
        return success();
    }
};

using MeasureQubitOpLowering =
    OpLoweringTemplate<qnet::MeasureOp, mlir::lir::MeasureOp>;

using NewQubitOpLowering =
    OpLoweringTemplate<qnet::NewQubitOp, mlir::lir::NewQubitOp>;

using RotZOpLowering = OpLoweringTemplate<qnet::RotZOp, mlir::lir::RotateZOp>;

// TODO - instantiate the template to map operations from one dialect to the
// other

} // namespace qoala::conversion

#endif // QNET_TO_QMEM_PATTERNS:_H
