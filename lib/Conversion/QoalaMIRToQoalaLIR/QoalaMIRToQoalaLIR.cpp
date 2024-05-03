#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/Debug.h"

#include "Analysis/Helpers/Helpers.h"
#include "Analysis/QMem/Conversion.h"
#include "Conversion/Helpers/Helpers.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIRPatterns.h"

namespace mlir {
#define GEN_PASS_DEF_QOALAMIRTOQOALALIR
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h.inc"
} // namespace mlir

#define DEBUG_TYPE "mir-to-lir"

using namespace mlir;
using namespace llvm;
using namespace qoala::helpers;
using namespace qoala::dialects;
using namespace qoala::helpers::angle;

/* Function copied from the "simple" functionization PoC. Since we will modify how we "group" operations
 * to create the new functions, this function will disappear */
static bool qMemOpCanBeFunctionized(mlir::Operation *op) {
    // A list fo the QMem operation types we would like to "functionize"
    return llvm::isa<
            qmem::CnotOp,
            qmem::CzOp,
            qmem::EprsMeasureOp,
            qmem::EprsOp,
            qmem::HadamardOp,
            qmem::InitOp,
            qmem::MeasureOp,
            qmem::QAllocOp,
            // Recv/Send Ints/Floats can stay in the main body
//            qmem::RecvFloatsOp,
//            qmem::RecvIntsOp,
//            qmem::SendFloatsOp,
//            qmem::SendIntsOp,
            // We do not want to functionize any leftover rotation (if any)
            // Since rotations use an f32 angle arg, they are lowered to intermediate rotations
            // (so the f32 can be transformed into 2x i32),THEN to netqasm rotations
            // This allows us to place the call to the builtin angle conversion function before functionizing
//            qmem::RotateXOp,
//            qmem::RotateYOp,
//            qmem::RotateZOp,
//            qmem::CrotXOp
            // Instead, we need to functionize the
            qmem::RotateXIntOp,
            qmem::RotateYIntOp,
            qmem::RotateZIntOp,
            qmem::CrotXIntOp
            // We don't want to functionize "Remotes", "Funcs" nor "Returns"
//            qmem::RemoteOp,
//            qmem::FuncOp,
//            qmem::ReturnOp,
    >(op);
}

namespace qoala::conversion {
    class QoalaMIRToQoalaLIRPass : public mlir::impl::QoalaMIRToQoalaLIRBase<QoalaMIRToQoalaLIRPass> {
        void runOnOperation() override;
    };

    void QoalaMIRToQoalaLIRPass::runOnOperation() {
        MLIRContext &context = this->getContext();
        ModuleOp module = dyn_cast<ModuleOp>(this->getOperation());
        assert(module);
        LLVM_DEBUG(llvm::dbgs() << "Converting MIR to LIR on module\n");

        // Get a conversion qMemLoweringTarget to define our qMemLoweringTarget dialects
        ConversionTarget qMemLoweringTarget(context);
        qMemLoweringTarget.addLegalDialect<qoalahost::QoalaHostDialect>();
        qMemLoweringTarget.addLegalDialect<netqasm::NetQASMDialect>();
        qMemLoweringTarget.addIllegalDialect<qmem::QMemDialect>();
        // There are NO legal options in QMem dialect after this pass
        //qMemLoweringTarget.addLegalOp<
            // some::Class
        //>();

        NullTypeConverter typeConverter(&context);

        ConversionTarget f32LoweringTarget(context);
        qoala::helpers::configureF32LoweringTarget(f32LoweringTarget);
        RewritePatternSet f32Patterns(&context);
        populateQMemF32ToInt32RotPatterns(context, f32Patterns, typeConverter);

        // We add the conversion pattern to the context
        RewritePatternSet allPatterns(&context);
        populateQMemToQoalaHostPatterns(context, allPatterns, typeConverter);
        populateQMemToNetQASMPatterns(context, allPatterns, typeConverter);

        if (!moduleContainsAngleConversionDeclaration(module)) {
            insertAngleConversionFunctionDeclaration(module);
        }

        // Stage 1: Transform f32 type - This is done with an "intra-dialect" lowering
        LogicalResult f32LoweringResult =
                mlir::applyPartialConversion(module, f32LoweringTarget, std::move(f32Patterns));
        if (mlir::failed(f32LoweringResult)) {
            signalPassFailure();
        }

        // Stage 2: Functionize
        qoala::analysis::functionize::functionizeModule(module, qMemOpCanBeFunctionized);
        module.dump();

        // Stage 3: Apply the QMemtoNetQASM and QMemToQoalaHost conversion patterns
        LogicalResult conversionResult =
            mlir::applyPartialConversion(module, qMemLoweringTarget, std::move(allPatterns));
        if (mlir::failed(conversionResult)) {
            signalPassFailure();
        }
    }
} /* namespace qoala::conversion */

std::unique_ptr<mlir::Pass> mlir::createQoalaMIRToQoalaLIRPass() {
    return std::make_unique<qoala::conversion::QoalaMIRToQoalaLIRPass>();
}
