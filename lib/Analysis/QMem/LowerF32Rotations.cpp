#include "mlir/IR/BuiltinOps.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "Dialect/Helpers/MIRToLIRHelperPasses.h"
#include "Dialect/QMem/QMem.h"

#include "Analysis/Helpers/Helpers.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIRPatterns.h"

#include "llvm/Support/Debug.h"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::helpers;
using namespace qoala::analysis;
using namespace qoala::conversion;

#define DEBUG_TYPE "f32-flattening"

namespace qoala::helpers {
    void configureF32LoweringTarget(ConversionTarget &target) {
        // When converting F32 to I32 rotations, configure that after conversion all QMem operations are valid
        target.addLegalDialect<qmem::QMemDialect>();
        // ... EXCEPT for the ones we want to convert
        target.addIllegalOp<
                qmem::RotateXOp,
                qmem::RotateYOp,
                qmem::RotateZOp,
                qmem::CrotXOp
        >();
        // Call ops ARE ALSO allowed in this conversion
        target.addLegalOp<func::CallOp>();
    }


    void populateQMemF32ToInt32RotPatterns(
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
#define GEN_PASS_DEF_CONVERTINTEGERTOFLOATROTATIONS
#include "Dialect/Helpers/HelperPasses.h.inc"

    class LowerF32RotationsPass : public impl::ConvertIntegerToFloatRotationsBase<LowerF32RotationsPass> {
    public:
        using ConvertIntegerToFloatRotationsBase::ConvertIntegerToFloatRotationsBase;
        void runOnOperation() override;
    };

    void LowerF32RotationsPass::runOnOperation() {
        qmem::FuncOp mainFuncOp = this->getOperation();
        MLIRContext &context = this->getContext();
        LLVM_DEBUG(llvm::dbgs() << "Lowering f32 rotation operations\n");

        ConversionTarget f32LoweringTarget(context);
        configureF32LoweringTarget(f32LoweringTarget);

        RewritePatternSet f32Patterns(&context);
        NullTypeConverter typeConverter(&context);
        populateQMemF32ToInt32RotPatterns(context, f32Patterns, typeConverter);

        if (failed(applyPartialConversion(mainFuncOp, f32LoweringTarget, std::move(f32Patterns)))) {
            signalPassFailure();
        }
    }
} /* namespace qoala::analysis */
