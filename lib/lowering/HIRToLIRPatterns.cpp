#include "mlir/Transforms/DialectConversion.h"
#include "lowering/HIRToLIRPatterns.h"
#include "llvm/Support/Debug.h"
#include "Dialect/hir/Hir.h"
#include "Dialect/lir/Lir.h"

qoala::lowering::HirQubitToLirQubitTypeConverter::HirQubitToLirQubitTypeConverter(MLIRContext *ctx) {
    // Default conversion for non hir::QubitType instances
    addConversion([](Type type) { return type; });
    addConversion([ctx](hir::QubitType type) -> Type {
        return lir::QubitType::get(ctx);
    });
    // In case there will be illegal values (values of types declared illegal) after conversion, then we
    // need to provide a "materialization", i.e. how to cast from one type to the other
    // Here we provide a "cast" to transform from source type (hir::QubitType) to target type (lir::QubitType)
    addTargetMaterialization([&](
            OpBuilder &builder, Type resultType,
            ValueRange inputs, Location loc) -> Value {
        //The conversion is simply a "builtin::unrealized_cast" operation
        return builder.create<UnrealizedConversionCastOp>(loc, resultType, inputs).getResult(0);
    });
    // Here we provide a "cast" to transform from target type (lir::QubitType) to source type (hir::QubitType)
    addSourceMaterialization([&](
            OpBuilder &builder, Type resultType,
            ValueRange inputs, Location loc) -> std::optional<Value> {
        //The conversion is simply a "builtin::unrealized_cast" operation
        return builder.create<UnrealizedConversionCastOp>(loc, resultType, inputs).getResult(0);
    });
}

LogicalResult qoala::lowering::NewQubitOpLowering::matchAndRewrite(
        hir::NewQubitOp op, mlir::hir::NewQubitOp::Adaptor adaptor,
        ConversionPatternRewriter &rewriter) const {
    llvm::dbgs() << "lowering operation : '" << op << "'\n";
    auto loc = op->getLoc();

    auto lirNewQubit = rewriter.create<lir::NewQubitOp>(loc);
    rewriter.replaceOp(op.getOperation(), lirNewQubit);
    return success();
}

LogicalResult qoala::lowering::MeasureQubitOpLowering::matchAndRewrite(
        hir::MeasureOp op, OpAdaptor adaptor,
        ConversionPatternRewriter &rewriter) const {
    llvm::dbgs() << "lowering operation : '" << op << "'\n";
    auto newOp = rewriter.replaceOpWithNewOp<lir::MeasureOp>(op, adaptor.getOperands()[0]);
    return success();
}
