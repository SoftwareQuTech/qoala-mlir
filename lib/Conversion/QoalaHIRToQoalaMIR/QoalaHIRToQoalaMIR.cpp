#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"

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
        Operation *operation = getOperation();
        // Get a conversion target to define our target dialects
        ConversionTarget target(context);
        // We add the legal dialects that we aim to keep in the target
        target.addLegalDialect<qmem::QMemDialect>();
        // We define the QNet dialect as "illegal", so the conversion will fail
        // if there are any qnet operations in the converted IR
        target.addIllegalDialect<qnet::QNetDialect>();
        // We also declare operations (classes) that can be declared legal in the target
        // dialect. The `callback` argument (which receives the operation involved)
        // can determine if it is legal to leave the operation or not.
        /*
        target.addLegalOp<
                //qnet::SomeOp,
                //qnet::SomeOtherOp
        >();
         */

        // TODO - Add lowering for Tensor -> Memref (or potentially one level down)

        // We add the conversion pattern to the context
        RewritePatternSet patterns(&context);
        hir::QoalaHIRToQoalaMIRTypeConverter typeConverter(&context);
        patterns.add<
                hir::FuncOpLowering,
                hir::ReturnOpLowering,
                hir::EprsOpLowering,
                hir::EprsMeasureOpLowering,
                hir::NewQubitLowering,
                hir::RemoteOpLowering,
                hir::RecvIntsOpLowering,
                hir::RecvFloatsOpLowering,
                hir::SendIntsOpLowering,
                hir::SendFloatsOpLowering,
                hir::RotateXLowering,
                hir::RotateYLowering,
                hir::RotateZLowering,
                hir::HadamardLowering,
                hir::CNotLowering,
                hir::CzLowering,
                hir::CRotXLowering,
                hir::MeasureLowering
        >(typeConverter, &context);

        // We finally apply a **partial** conversion, since there will be some
        // operations that will stay... momentarily
        LogicalResult result =
            mlir::applyPartialConversion(operation, target, std::move(patterns));
        if (mlir::failed(result)) {
            signalPassFailure();
        }
    }
} /* namespace qoala::conversion */
