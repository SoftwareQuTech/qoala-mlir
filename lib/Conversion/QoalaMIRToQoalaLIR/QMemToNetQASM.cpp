#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/Debug.h"

#include "Analysis/Helpers/Helpers.h"
#include "Conversion/Helpers/Helpers.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIRPatterns.h"

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
#define GEN_PASS_DEF_LOWERQMEMTONETQASM
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h.inc"

    class LowerQMemToNetQASMPass : public impl::LowerQMemToNetQASMBase<LowerQMemToNetQASMPass> {
        using LowerQMemToNetQASMBase::LowerQMemToNetQASMBase;
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

        // Lowering QMem to NetQASM expect to lower rotations. so we need to lower the
        // f32 rotations using the intermediate step
        ConversionTarget f32LoweringTarget(context);
        RewritePatternSet f32Patterns(&context);
        qoala::helpers::configureF32LoweringTarget(f32LoweringTarget);
        qoala::helpers::populateQMemF32ToInt32RotPatterns(context, f32Patterns, typeConverter);

        if (!moduleContainsAngleConversionDeclaration(module)) {
            insertAngleConversionFunctionDeclaration(module);
        }

        // First, lower f32 rotations
        LogicalResult f32LoweringResult =
                mlir::applyPartialConversion(module, f32LoweringTarget, std::move(f32Patterns));
        if (mlir::failed(f32LoweringResult)) {
            signalPassFailure();
        }

        // Then, lower the rest
        LogicalResult result =
                mlir::applyPartialConversion(module, target, std::move(patterns));
        if (mlir::failed(result)) {
            signalPassFailure();
        }
    }
} /* namespace qoala::conversion */

