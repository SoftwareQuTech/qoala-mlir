#include "llvm/Support/Debug.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/IR/BuiltinDialect.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"

#define DEBUG_TYPE "QoalaHIRToQoalaMIR"

#include "Conversion/QoalaHIRToQoalaMIR/QoalaHIRToQoalaMIR.h"
#include "Conversion/QoalaHIRToQoalaMIR/QoalaHIRToQoalaMIRPatterns.h"

using namespace mlir;
using namespace llvm;
using namespace qoala::helpers;
using namespace qoala::dialects;

namespace qoala::conversion {
#define GEN_PASS_DEF_QOALAHIRTOQOALAMIR
#include "Conversion/QoalaHIRToQoalaMIR/QoalaHIRToQoalaMIR.h.inc"

    class QoalaHIRToQoalaMIRPass : public impl::QoalaHIRToQoalaMIRBase<QoalaHIRToQoalaMIRPass> {
        using QoalaHIRToQoalaMIRBase::QoalaHIRToQoalaMIRBase;
        void runOnOperation() override;
    };

    void QoalaHIRToQoalaMIRPass::runOnOperation() {
        MLIRContext &context = getContext();
        ModuleOp module = getOperation();
        // Get a conversion target to define our target dialects
        ConversionTarget target(context);
        // We add the legal dialects that we aim to keep in the target
        target.addLegalDialect<qmem::QMemDialect>();
        target.addLegalDialect<BuiltinDialect>();
        target.addLegalDialect<arith::ArithDialect>();
        target.addLegalDialect<tensor::TensorDialect>();
        // We define the QNet dialect as "illegal", so the conversion will fail
        // if there are any qnet operations in the converted IR
        target.addIllegalDialect<qnet::QNetDialect>();
        // We also declare operations (classes) that can be declared legal in the target
        // dialect.
        // We can also use the `addDynamicallyLegalOp` method to determine at runtime if
        // an operation is legal. The `callback` argument (which receives the operation involved)
        // can determine if it is legal to leave the operation or not.
        /*
        target.addLegalOp<
                //qnet::SomeOp,
                //qnet::SomeOtherOp
        >();
         */
        // For some reason, declaring the UnrealizedConversionCastOp operation as illegal in the
        // target has no effect. (i.e. the IR will be tagged as valid, despite still having some
        // UnrealizedConversionCastOp instances)
        // target.addIllegalOp<UnrealizedConversionCastOp>();

        // We add the conversion pattern to the context
        RewritePatternSet patterns(&context);
        hir::QoalaHIRToQoalaMIRTypeConverter typeConverter(&context);
        patterns.add<hir::FuncOpLowering, hir::ReturnOpLowering, hir::EprsOpLowering, hir::EprsMeasureOpLowering,
                     hir::NewQubitLowering, hir::RemoteOpLowering, hir::RecvIntsOpLowering, hir::RecvFloatsOpLowering,
                     hir::SendIntsOpLowering, hir::SendFloatsOpLowering, hir::RecvIntOpLowering,
                     hir::RecvFloatOpLowering, hir::SendIntOpLowering, hir::SendFloatOpLowering, hir::RotateXLowering,
                     hir::RotateYLowering, hir::RotateZLowering, hir::HadamardLowering, hir::CNotLowering,
                     hir::CzLowering, hir::CRotXLowering, hir::MeasureLowering>(typeConverter, &context);

        // We apply a **full** conversion, since we correctly defined all the
        // dialects that are "legal" in the target IR
        if (failed(applyFullConversion(module.getOperation(), target, std::move(patterns)))) {
            LLVM_DEBUG(llvm::dbgs() << module << "\n");
            signalPassFailure();
        }
        // Finally, we have to get rid of any leftover UnrealizedConversionCastOp... provided that
        // they don't have any uses. If we find one that has one or more uses, then the dialect
        // conversion process is bugged.
        // NB: This is an ugly, yet effective solution.
        WalkResult result = module.walk([](const UnrealizedConversionCastOp leftoverCast) -> WalkResult {
            if (leftoverCast->getUses().empty()) {
                leftoverCast->erase();
                return WalkResult::advance();
            }
            leftoverCast->emitOpError("Leftover unrealized cast still has usages. ")
                    << "This is a bug on the implementation of the HIR to MIR lowering.";
            return WalkResult::interrupt();
        });

        if (result.wasInterrupted()) {
            signalPassFailure();
        }
    }
} /* namespace qoala::conversion */
