#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/Debug.h"

#include "Analysis/Helpers/Helpers.h"
#include "Conversion/Helpers/Helpers.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h"

namespace mlir {
#define GEN_PASS_DEF_LOWERQMEMTOQOALAHOST
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h.inc"
} // namespace mlir

#define DEBUG_TYPE "qmem-to-qoalahost"

using namespace mlir;
using namespace llvm;
using namespace qoala::helpers;
using namespace qoala::dialects;
using namespace qoala::helpers::angle;

namespace qoala::helpers {
    void populateQMemToQoalaHostPatterns(
            MLIRContext &context, RewritePatternSet &patterns,
            TypeConverter &typeConverter) {
//        patterns.add<
        // TODO - To Implement
//        >(typeConverter, context);
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
        target.addLegalDialect<netqasm::NetQASMDialect>();
        target.addIllegalDialect<qoalahost::QoalaHostDialect>();
        target.addLegalOp<
                // Everything BUT qmem.func amd qmem.return is legal after this pass,
                // because this pass does not modify these operations
                qmem::CnotOp,
                qmem::CrotXOp,
                qmem::CzOp,
                qmem::EprsMeasureOp,
                qmem::EprsOp,
                qmem::HadamardOp,
                qmem::InitOp,
                qmem::MeasureOp,
                qmem::QAllocOp,
                qmem::RecvFloatsOp,
                qmem::RecvIntsOp,
                qmem::RemoteOp,
                qmem::RotateXOp,
                qmem::RotateYOp,
                qmem::RotateZOp,
                qmem::SendFloatsOp,
                qmem::SendIntsOp
        >();

        // We add the conversion pattern to the context
        RewritePatternSet patterns(&context);
        // We don't need a type converter in this stage
        NullTypeConverter typeConverter(&context);
        populateQMemToQoalaHostPatterns(context, patterns, typeConverter);

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
