#include "Conversion/QNetToQMem/QNetToQMemPatterns.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/Dialect/Arith/IR/Arith.h"

namespace qoala::conversion {
    /* Implementation of the qoala types converter */
    QNetToQMemQubitTypeConverter::QNetToQMemQubitTypeConverter(MLIRContext *ctx) {
        // Default conversion for non qnet::QubitType instances
        addConversion([](Type type) { return type; });
        addConversion([ctx](qnet::QubitType type) -> Type {
            // Qubit Types are mapped into i32 (pointers)
            return IntegerType::get(ctx, 32);
        });
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

    /* Implementation of the lowering for the creation of new qubits */
    qmem::QAllocOp
    NewQubitLowering::createNewOp(qnet::NewQubitOp op, qnet::NewQubitOp::Adaptor adaptor,
                                  ConversionPatternRewriter &rewriter) const {
        Location loc = op.getLoc();
        // First, we convert the qnet.qubit type into its mapped type
        Type mappedQubitType = typeConverter->convertType(op.getQout().getType());

        // Second, we create a QAllocOp instance that allocates a single qubit
        auto newAllocOp = rewriter.create<qmem::QAllocOp>(loc, mappedQubitType);

        // Third, we introduce the "init" operation required by the allocation
        auto newInitOp = rewriter.create<qmem::InitOp>(loc, newAllocOp.getResult());
        return dyn_cast<qmem::QAllocOp>(newInitOp.getQ().getDefiningOp());
    }

    RotateXLowering::NewOpAndValues
    RotateXLowering::createNewOpAndValues(
            qnet::RotXOp op, qnet::RotXOp::Adaptor adaptor, ConversionPatternRewriter &rewriter) const {
        llvm::dbgs() << "lowering operation : '" << op << "'\n";
        rewriter.replaceAllUsesWith(op.getQout(), adaptor.getQin());
        auto newRotate = rewriter.create<qmem::RotateXOp>(op.getLoc(), adaptor.getQin(), adaptor.getAngle());
        // This is a tricky replacement.... we need to replace the operation *WITH THE VALUE*
        return RotateXLowering::NewOpAndValues(newRotate, ValueRange{newRotate.getQ()});
    }

/* Implementation of the specific conversion between similar ops */
    qmem::RemoteOp
    RemoteOpLowering::createNewOp(qnet::RemoteOp op, qnet::RemoteOp::Adaptor adaptor,
                                  ConversionPatternRewriter &rewriter) const {
        return rewriter.create<qmem::RemoteOp>(op.getLoc(), adaptor.getSymName(), adaptor.getSymVisibilityAttr());
    }

    qmem::SendIntsOp
    SendIntsOpLowering::createNewOp(qnet::SendIntsOp op, qnet::SendIntsOp::Adaptor adaptor,
                                    ConversionPatternRewriter &rewriter) const {
        return rewriter.create<qmem::SendIntsOp>(op.getLoc(), adaptor.getCin(), adaptor.getRemoteAttr());
    }

    qmem::RecvIntsOp
    RecvIntsOpLowering::createNewOp(qnet::RecvIntsOp op, qnet::RecvIntsOp::Adaptor adaptor,
                                    ConversionPatternRewriter &rewriter) const {
        return rewriter.create<qmem::RecvIntsOp>(op.getLoc(), adaptor.getOperands(), adaptor.getRemoteAttr());
    }

    qmem::SendFloatsOp
    SendFloatsOpLowering::createNewOp(qnet::SendFloatsOp op, qnet::SendFloatsOp::Adaptor adaptor,
                                      ConversionPatternRewriter &rewriter) const {
        return rewriter.create<qmem::SendFloatsOp>(op.getLoc(), adaptor.getCin(), adaptor.getRemoteAttr());
    }

    qmem::RecvFloatsOp
    RecvFloatsOpLowering::createNewOp(qnet::RecvFloatsOp op, qnet::RecvFloatsOp::Adaptor adaptor,
                                      ConversionPatternRewriter &rewriter) const {
        return rewriter.create<qmem::RecvFloatsOp>(op.getLoc(), adaptor.getOperands(), adaptor.getRemoteAttr());
    }
} // namespace qoala::conversion