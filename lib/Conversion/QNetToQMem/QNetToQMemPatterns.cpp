#include "Conversion/QNetToQMem/QNetToQMemPatterns.h"

/* Implementation of the qoala types converter */
qoala::conversion::QNetToQMemQubitTypeConverter::QNetToQMemQubitTypeConverter(MLIRContext *ctx) {
    // Default conversion for non qnet::QubitType instances
    addConversion([](Type type) { return type; });
    // TODO - The QbitType of QNet does not have a similar type on QMem... It needs to be lowered to Memrefs
//    addConversion([ctx](qnet::QubitType type) -> Type {
//        return lir::QubitType::get(ctx);
//    });
    // In case there will be illegal values (values of types declared illegal)
    // after conversion, then we need to provide a "materialization", i.e. how
    // to cast from one type to the other Here we provide a "cast" to transform
    // from source type (qnet::QubitType) to target type (lir::QubitType)
    addTargetMaterialization([&](OpBuilder &builder, Type resultType,
                                 ValueRange inputs, Location loc) -> Value {
        // The conversion is simply a "builtin::unrealized_cast" operation
        return builder
            .create<UnrealizedConversionCastOp>(loc, resultType, inputs)
            .getResult(0);
    });
    // Here we provide a "cast" to transform from target type (lir::QubitType)
    // to source type (qnet::QubitType)
    addSourceMaterialization([&](OpBuilder &builder, Type resultType,
                                 ValueRange inputs,
                                 Location loc) -> std::optional<Value> {
        // The conversion is simply a "builtin::unrealized_cast" operation
        return builder
            .create<UnrealizedConversionCastOp>(loc, resultType, inputs)
            .getResult(0);
    });
}

/* Implementation of the specific conversion between similar ops */
qmem::RemoteOp
qoala::conversion::RemoteOpLowering ::createNewOp(qnet::RemoteOp op, qnet::RemoteOp::Adaptor adaptor,
                                                  ConversionPatternRewriter &rewriter) const {
    return rewriter.create<qmem::RemoteOp>(op.getLoc(), adaptor.getSymName(), adaptor.getSymVisibilityAttr());
}

qmem::SendIntsOp
qoala::conversion::SendIntsOpLowering::createNewOp(qnet::SendIntsOp op, qnet::SendIntsOp::Adaptor adaptor,
                                                   ConversionPatternRewriter &rewriter) const {
    return rewriter.create<qmem::SendIntsOp>(op.getLoc(), adaptor.getCin(), adaptor.getRemoteAttr());
}

qmem::RecvIntsOp
qoala::conversion::RecvIntsOpLowering::createNewOp(qnet::RecvIntsOp op, qnet::RecvIntsOp::Adaptor adaptor,
                                                   ConversionPatternRewriter &rewriter) const {
    return rewriter.create<qmem::RecvIntsOp>(op.getLoc(), adaptor.getOperands(), adaptor.getRemoteAttr());
}

qmem::SendFloatsOp
qoala::conversion::SendFloatsOpLowering::createNewOp(qnet::SendFloatsOp op, qnet::SendFloatsOp::Adaptor adaptor,
                                                     ConversionPatternRewriter &rewriter) const {
    return rewriter.create<qmem::SendFloatsOp>(op.getLoc(), adaptor.getCin(), adaptor.getRemoteAttr());
}

qmem::RecvFloatsOp
qoala::conversion::RecvFloatsOpLowering::createNewOp(qnet::RecvFloatsOp op, qnet::RecvFloatsOp::Adaptor adaptor,
                                                     ConversionPatternRewriter &rewriter) const {
    return rewriter.create<qmem::RecvFloatsOp>(op.getLoc(), adaptor.getOperands(), adaptor.getRemoteAttr());
}
