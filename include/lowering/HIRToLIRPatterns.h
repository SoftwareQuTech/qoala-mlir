#ifndef QOALA_MLIR_HIRTOLIRPATTERNS_H
#define QOALA_MLIR_HIRTOLIRPATTERNS_H
#include "mlir/Transforms/DialectConversion.h"
#include "Dialect/hir/Hir.h"

namespace qoala::lowering {
class HirQubitToLirQubitTypeConverter : public TypeConverter {
public:
    explicit HirQubitToLirQubitTypeConverter(MLIRContext *ctx);
};

class NewQubitOpLowering : public OpConversionPattern<mlir::hir::NewQubitOp> {
public:
    NewQubitOpLowering(const TypeConverter &typeConverter, MLIRContext *ctx)
            : OpConversionPattern(typeConverter, ctx) { }
    LogicalResult matchAndRewrite(
            mlir::hir::NewQubitOp op, mlir::hir::NewQubitOp::Adaptor adaptor,
            ConversionPatternRewriter &rewriter) const override;
};

class MeasureQubitOpLowering : public OpConversionPattern<mlir::hir::MeasureOp> {
public:
    MeasureQubitOpLowering(const TypeConverter &typeConverter, MLIRContext *ctx)
            : OpConversionPattern(typeConverter, ctx) { }
    LogicalResult matchAndRewrite(
            mlir::hir::MeasureOp op, OpAdaptor adaptor,
            ConversionPatternRewriter &rewriter) const override;
};
} //namespace qoala::lowering

#endif //QOALA_MLIR_HIRTOLIRPATTERNS_H
