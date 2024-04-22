#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"

#include "Conversion/Helpers/Helpers.h"
#include "Conversion/QoalaHIRToQoalaMIR/QoalaHIRToQoalaMIR.h"
#include "Conversion/QoalaHIRToQoalaMIR/QoalaHIRToQoalaMIRPatterns.h"

namespace mlir {
#define GEN_PASS_DEF_QOALAHIRTOQOALAMIR
#include "Conversion/QoalaHIRToQoalaMIR/QoalaHIRToQoalaMIR.h.inc"
} // namespace mlir

using namespace mlir;
using namespace llvm;
using namespace qoala::helpers;
using namespace qoala::dialects;

namespace qoala::conversion {
    class QoalaHIRToQoalaMIRPass : public mlir::impl::QoalaHIRToQoalaMIRBase<QoalaHIRToQoalaMIRPass> {
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

        // We add the conversion pattern to the context
        RewritePatternSet patterns(&context);
        QoalaHIRToQoalaMIRTypeConverter typeConverter(&context);
        patterns.add<
                FuncOpLowering,
                ReturnOpLowering,
                EprsOpLowering,
                EprsMeasureOpLowering,
                NewQubitLowering,
                RemoteOpLowering,
                RecvIntsOpLowering,
                RecvFloatsOpLowering,
                SendIntsOpLowering,
                SendFloatsOpLowering,
                RotateXLowering,
                RotateYLowering,
                RotateZLowering,
                HadamardLowering,
                CNotLowering,
                CzLowering,
                CRotXLowering,
                MeasureLowering
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

std::unique_ptr<mlir::Pass> mlir::createQoalaHIRToQoalaMIRPass() {
    return std::make_unique<qoala::conversion::QoalaHIRToQoalaMIRPass>();
}
