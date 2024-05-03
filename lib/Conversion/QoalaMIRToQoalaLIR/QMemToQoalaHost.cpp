#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/Debug.h"

#include "Analysis/Helpers/Helpers.h"
#include "Conversion/Helpers/Helpers.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIRPatterns.h"

namespace mlir {
#define GEN_PASS_DEF_LOWERQMEMTOQOALAHOST
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h.inc"
} // namespace mlir

#define DEBUG_TYPE "qmem-to-qoalahost"

using namespace mlir;
using namespace llvm;
using namespace qoala::helpers;
using namespace qoala::dialects;
using namespace qoala::conversion;
using namespace qoala::helpers::angle;

namespace qoala::helpers {
    void configureQMemToQoalaHostTarget(ConversionTarget &target,
                                        bool intRotsAreLegal,
                                        bool floatRotsAreLegal) {
        target.addLegalDialect<qoalahost::QoalaHostDialect>();
        target.addIllegalDialect<qmem::QMemDialect>();
        target.addLegalOp<
                // Everything BUT qmem.func amd qmem.return is legal after this pass,
                // because this pass does not modify these operations
                qmem::CnotOp,
                qmem::CzOp,
                qmem::EprsMeasureOp,
                qmem::EprsOp,
                qmem::HadamardOp,
                qmem::InitOp,
                qmem::MeasureOp,
                qmem::QAllocOp
        >();
        if (intRotsAreLegal) {
            target.addLegalOp<
                    qmem::RotateXIntOp,
                    qmem::RotateYIntOp,
                    qmem::RotateZIntOp,
                    qmem::CrotXIntOp
            >();
        } else {
            target.addIllegalOp<
                    qmem::RotateXIntOp,
                    qmem::RotateYIntOp,
                    qmem::RotateZIntOp,
                    qmem::CrotXIntOp
            >();
        }
        if (floatRotsAreLegal) {
            target.addLegalOp<
                    qmem::RotateXOp,
                    qmem::RotateYOp,
                    qmem::RotateZOp,
                    qmem::CrotXOp
            >();
        } else {
            target.addIllegalOp<
                    qmem::RotateXOp,
                    qmem::RotateYOp,
                    qmem::RotateZOp,
                    qmem::CrotXOp
            >();
        }
    }

    void populateQMemToQoalaHostPatterns(
            MLIRContext &context, RewritePatternSet &patterns,
            TypeConverter &typeConverter) {
        patterns.add<
                mir::RemoteOpLowering,
                mir::FuncOpLowering,
                mir::ReturnOpLowering,
                mir::CallOpLowering,
                mir::RecvIntsOpLowering,
                mir::RecvFloatsOpLowering,
                mir::SendIntsOpLowering,
                mir::SendFloatsOpLowering
        >(typeConverter, &context);
    }
}

namespace qoala::conversion {
    class LowerQMemToQoalaHostPass : public mlir::impl::LowerQMemToQoalaHostBase<LowerQMemToQoalaHostPass> {
        void runOnOperation() override;
    };

    void LowerQMemToQoalaHostPass::runOnOperation() {
        MLIRContext &context = this->getContext();
        ModuleOp module = dyn_cast<ModuleOp>(this->getOperation());
        assert(module);
        LLVM_DEBUG(llvm::dbgs() << "Lowering QMem to QoalaHost on module\n");

        ConversionTarget target(context);
        // We add the conversion pattern to the context
        RewritePatternSet patterns(&context);
        // We don't need a type converter in this stage
        NullTypeConverter typeConverter(&context);

        qoala::helpers::configureQMemToQoalaHostTarget(target, true, true);
        qoala::helpers::populateQMemToQoalaHostPatterns(context, patterns, typeConverter);

        if (!moduleContainsAngleConversionDeclaration(module)) {
            insertAngleConversionFunctionDeclaration(module);
        }

        LogicalResult result =
                mlir::applyPartialConversion(module, target, std::move(patterns));
        if (mlir::failed(result)) {
            signalPassFailure();
        }
    }
}

std::unique_ptr<mlir::Pass> mlir::createLowerQMemToQoalaHostPass() {
    return std::make_unique<qoala::conversion::LowerQMemToQoalaHostPass>();
}
