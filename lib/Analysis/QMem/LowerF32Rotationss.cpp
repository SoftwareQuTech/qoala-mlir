#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Transforms/DialectConversion.h"
#include "Dialect/QMem/Passes.h"
#include "Dialect/QMem/QMem.h"

#include "Analysis/QMem/Conversion.h"
#include "Analysis/Helpers/Helpers.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIRPatterns.h"

#include "llvm/Support/Debug.h"

namespace mlir {
#define GEN_PASS_DEF_QMEMCONVERTINTEGERTOFLOATROTATIONS
#include "Dialect/QMem/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::helpers;
using namespace qoala::analysis;
using namespace qoala::conversion;

#define DEBUG_TYPE "f32-flattening"

static bool operationUsesF32Angle(Operation *operation) {
    return isa<
            qmem::RotateXOp,
            qmem::RotateYOp,
            qmem::RotateZOp,
            qmem::CrotXOp
    >(operation);
}

namespace qoala::helpers {
    void populateQMemToIntermediateQMemPatterns(
            MLIRContext &context, RewritePatternSet &patterns,
            TypeConverter &typeConverter) {
        patterns.add<
                mir::RotateXLowering,
                mir::RotateYLowering,
                mir::RotateZLowering,
                mir::CRotXLowering
        >(typeConverter, &context);
    }
}

namespace qoala::analysis {
    class LowerF32RotationsPass : public impl::QMemConvertIntegerToFloatRotationsBase<LowerF32RotationsPass> {
    public:
        void runOnOperation() override;
    };

    void LowerF32RotationsPass::runOnOperation() {
        ModuleOp module = dyn_cast<ModuleOp>(this->getOperation());
        assert(module); // We expect the cast to succeed
        MLIRContext &context = this->getContext();
        LLVM_DEBUG(llvm::dbgs() << "Lowering f32 rotation operations\n");

        ConversionTarget f32LoweringTarget(context);
        f32LoweringTarget.addIllegalOp<
                qmem::RotateXOp,
                qmem::RotateYOp,
                qmem::RotateZOp,
                qmem::CrotXOp
        >();
        RewritePatternSet f32Patterns(&context);
        NullTypeConverter typeConverter(&context);
        populateQMemToIntermediateQMemPatterns(context, f32Patterns, typeConverter);

        LogicalResult f32ConversionResult =
                mlir::applyPartialConversion(module, f32LoweringTarget, std::move(f32Patterns));
        if (mlir::failed(f32ConversionResult)) {
            signalPassFailure();
        }
    }
} /* namespace qoala::analysis */


std::unique_ptr<Pass> mlir::createQMemF32RotationsConversion() {
    return std::make_unique<qoala::analysis::LowerF32RotationsPass>();
}

