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
#define GEN_PASS_DEF_LOWERQMEMTONETQASM
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h.inc"
} // namespace mlir

#define DEBUG_TYPE "qmem-to-netqasm"

using namespace mlir;
using namespace llvm;
using namespace qoala::helpers;
using namespace qoala::dialects;
using namespace qoala::conversion;
using namespace qoala::helpers::angle;

namespace qoala::helpers {
    void populateQMemToNetQASMPatterns(
            MLIRContext &context, RewritePatternSet &patterns,
            TypeConverter &typeConverter) {
        patterns.add<
                mir::QAllocLowering,
                mir::QInitLowering,
                mir::RotateXLowering,
                mir::RotateYLowering,
                mir::RotateZLowering,
                mir::MeasureLowering,
                mir::HadamardLowering,
                mir::CNotLowering,
                mir::CzLowering,
                mir::CRotXLowering
        >(typeConverter, &context);
    }
}

namespace qoala::conversion {
    class LowerQMemToNetQASMPass : public mlir::impl::LowerQMemToNetQASMBase<LowerQMemToNetQASMPass> {
        void runOnOperation() override;
    };

    void LowerQMemToNetQASMPass::runOnOperation() {
        MLIRContext &context = this->getContext();
        ModuleOp module = dyn_cast<ModuleOp>(this->getOperation());
        assert(module);
        LLVM_DEBUG(llvm::dbgs() << "Lowering QMem to QoalaHost on module\n");

        ConversionTarget target(context);
        target.addLegalDialect<netqasm::NetQASMDialect>();
        target.addIllegalDialect<qmem::QMemDialect>();
        target.addLegalOp<
                // Only the "qmem.func" and "qmem.return" operations are legal after this pass,
                // because this pass modifies ALL the other operations but the main function
                qmem::FuncOp,
                qmem::ReturnOp
        >();

        // We add the conversion pattern to the context
        RewritePatternSet patterns(&context);
        // We don't need a type converter in this stage
        NullTypeConverter typeConverter(&context);
        qoala::helpers::populateQMemToNetQASMPatterns(context, patterns, typeConverter);

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

std::unique_ptr<mlir::Pass> mlir::createLowerQMemToNetQASMPass() {
    return std::make_unique<qoala::conversion::LowerQMemToNetQASMPass>();
}
