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
    void configureQMemToNetQASMTarget(ConversionTarget &target) {
        target.addLegalDialect<netqasm::NetQASMDialect>();
        target.addIllegalDialect<qmem::QMemDialect>();
        target.addLegalOp<
                // We declare as "legal" all the operations that directly map to QoalaHost operations
                qmem::FuncOp,
                qmem::ReturnOp,
                qmem::RemoteOp,
                qmem::RecvIntsOp,
                qmem::RecvFloatsOp,
                qmem::SendIntsOp,
                qmem::SendFloatsOp
        >();
    }
    void populateQMemToNetQASMPatterns(
            MLIRContext &context, RewritePatternSet &patterns,
            TypeConverter &typeConverter) {
        patterns.add<
                mir::MeasureOpLowering,
                mir::EprsOpLowering,
                mir::EprsMeasureOpLowering,
                mir::NetQASMFunctionLowering,
                mir::NetQASMReturnOpLowering,
                mir::QAllocLowering,
                mir::QInitLowering,
                mir::HadamardLowering,
                mir::CNotLowering,
                mir::CzLowering,
                mir::RotateXIntLowering,
                mir::RotateYIntLowering,
                mir::RotateZIntLowering,
                mir::CRotXIntLowering
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
        // We add the conversion pattern to the context
        RewritePatternSet patterns(&context);
        // We don't need a type converter in this stage
        NullTypeConverter typeConverter(&context);

        qoala::helpers::configureQMemToNetQASMTarget(target);
        qoala::helpers::populateQMemToNetQASMPatterns(context, patterns, typeConverter);

        // TODO - Add the "intermediate" lowering stage here

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
