#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/Debug.h"

#include "Analysis/Helpers/Helpers.h"
#include "Conversion/Helpers/Helpers.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIRPatterns.h"

#define DEBUG_TYPE "qmem-to-qoalahost"

using namespace mlir;
using namespace llvm;
using namespace qoala::helpers;
using namespace qoala::dialects;
using namespace qoala::conversion;
using namespace qoala::helpers::angle;

namespace qoala::helpers {
    void configureQMemToQoalaHostTarget(ConversionTarget &target,
                                        const bool intRotsAreLegal,
                                        const bool floatRotsAreLegal) {
        target.addLegalDialect<qoalahost::QoalaHostDialect>();
        target.addIllegalDialect<qmem::QMemDialect>();
        target.addLegalOp<
                // We declare as "legal" all the operations that directly map to NetQASM operations
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
#define GEN_PASS_DEF_LOWERQMEMTOQOALAHOST
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h.inc"

    class LowerQMemToQoalaHostPass : public impl::LowerQMemToQoalaHostBase<LowerQMemToQoalaHostPass> {
        using LowerQMemToQoalaHostBase::LowerQMemToQoalaHostBase;
        void runOnOperation() override;
    };

    void LowerQMemToQoalaHostPass::runOnOperation() {
        MLIRContext &context = this->getContext();
        ModuleOp module = this->getOperation();
        LLVM_DEBUG(llvm::dbgs() << "Lowering QMem to QoalaHost on module\n");

        ConversionTarget target(context);
        // We add the conversion pattern to the context
        RewritePatternSet patterns(&context);
        // We don't need a type converter in this stage
        NullTypeConverter typeConverter(&context);

        configureQMemToQoalaHostTarget(target, true, true);
        populateQMemToQoalaHostPatterns(context, patterns, typeConverter);

        if (!moduleContainsAngleConversionDeclaration(module)) {
            insertAngleConversionFunctionDeclaration(module);
        }

        if (failed(applyPartialConversion(module, target, std::move(patterns)))) {
            signalPassFailure();
        }
    }
} /* namespace qoala::conversion */
