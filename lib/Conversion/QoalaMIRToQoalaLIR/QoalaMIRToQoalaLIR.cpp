#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/Debug.h"

#include "Analysis/Helpers/Helpers.h"
#include "Analysis/QMem/Conversion.h"
#include "Conversion/Helpers/Helpers.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h"

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

static qoala::analysis::functionize::BucketsTy simpleOpClassifier(ModuleOp *module) {
    std::vector<Region *> foundRegion;
    module->walk([&](qmem::FuncOp mainFunc) {
        foundRegion.push_back(&mainFunc.getBody());
    });

    std::vector<std::vector<Operation *>> result;
    for (Operation &op : foundRegion[0]->getOps()) {
        if (qMemOpCanBeFunctionized(&op)) {
            std::vector<Operation *> intermediate;
            intermediate.push_back(&op);
            result.push_back(intermediate);
        }
    }
    return result;
}

namespace qoala::conversion {
#define GEN_PASS_DEF_QOALAMIRTOQOALALIR
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h.inc"

    class QoalaMIRToQoalaLIRPass : public impl::QoalaMIRToQoalaLIRBase<QoalaMIRToQoalaLIRPass> {
    public:
        using QoalaMIRToQoalaLIRBase::QoalaMIRToQoalaLIRBase;
        void runOnOperation() override;
    };

    void QoalaMIRToQoalaLIRPass::runOnOperation() {
        MLIRContext &context = this->getContext();
        ModuleOp module = dyn_cast<ModuleOp>(this->getOperation());
        assert(module);
        LLVM_DEBUG(llvm::dbgs() << "*************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* Converting MIR to LIR *\n");
        LLVM_DEBUG(llvm::dbgs() << "*************************\n");

        NullTypeConverter typeConverter(&context);

        ConversionTarget f32LoweringTarget(context);
        RewritePatternSet f32Patterns(&context);
        qoala::helpers::configureF32LoweringTarget(f32LoweringTarget);
        qoala::helpers::populateQMemF32ToInt32RotPatterns(context, f32Patterns, typeConverter);

        ConversionTarget qMemToQoalaHostTarget(context);
        RewritePatternSet qMemToQoalaHostPatterns(&context);
        qoala::helpers::configureQMemToQoalaHostTarget(qMemToQoalaHostTarget, true, false);
        qoala::helpers::populateQMemToQoalaHostPatterns(context, qMemToQoalaHostPatterns, typeConverter);

        ConversionTarget qMemToNetQASMTarget(context);
        RewritePatternSet qMemToNetQASMPatterns(&context);
        qoala::helpers::configureQMemToNetQASMTarget(qMemToNetQASMTarget);
        qoala::helpers::populateQMemToNetQASMPatterns(context, qMemToNetQASMPatterns, typeConverter);

        ConversionTarget qMemToQRemoteTarget(context);
        RewritePatternSet qMemToQRemotePatterns(&context);
        qoala::helpers::configureQMemToQRemoteTarget(qMemToQRemoteTarget);
        qoala::helpers::populateQMemToQRemotePatterns(context, qMemToQRemotePatterns, typeConverter);

        // Stage 1: Insert the declaration of the builtin angle conversion function
        LLVM_DEBUG(llvm::dbgs() << "*********************************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* 1. Inserting builtin function declaration *\n");
        LLVM_DEBUG(llvm::dbgs() << "*********************************************\n");
        if (!moduleContainsAngleConversionDeclaration(module)) {
            insertAngleConversionFunctionDeclaration(module);
        }

        // Stage 2: Transform f32 operations to their i32 counterparts - This is done with an "intra-dialect" lowering
        LLVM_DEBUG(llvm::dbgs() << "*****************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* 2. Lowering f32 rotations *\n");
        LLVM_DEBUG(llvm::dbgs() << "*****************************\n");
        LogicalResult f32LoweringResult =
                mlir::applyPartialConversion(module, f32LoweringTarget, std::move(f32Patterns));
        if (mlir::failed(f32LoweringResult)) {
            signalPassFailure();
        }

        // Stage 3: Functionize
        LLVM_DEBUG(llvm::dbgs() << "***************************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* 3. Functionizing quantum operations *\n");
        LLVM_DEBUG(llvm::dbgs() << "***************************************\n");

        if (this->useSimpleFunctionize) {
            LLVM_DEBUG(llvm::dbgs() << "HERE\n");
            qoala::analysis::functionize::functionizeModule(module, simpleOpClassifier);
        } else {
            // TODO
            qoala::analysis::functionize::functionizeModule(module, simpleOpClassifier);
        }
        // Correct the positions of the remote and builtin declaration
        module.walk([&](func::FuncOp funcDecl) {
            if (funcDecl.getSymName() != qoala::helpers::angle::angleConversionFunctionName) {
                WalkResult::advance();
            } else {
                qoala::helpers::moveOperationToTop(module, funcDecl);
            }
        });
        module.walk([&](qmem::RemoteOp remote) {
            qoala::helpers::moveOperationToTop(module, remote);
        });
        module.dump();

        // Stage 4: Transform f32 operations to their i32 counterparts - This is done with an "intra-dialect" lowering
        LLVM_DEBUG(llvm::dbgs() << "***********************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* 4. Lowering Remote declarations *\n");
        LLVM_DEBUG(llvm::dbgs() << "***********************************\n");
        LogicalResult remotesLoweringResult =
                mlir::applyPartialConversion(module, qMemToQRemoteTarget, std::move(qMemToQRemotePatterns));
        if (mlir::failed(remotesLoweringResult)) {
            signalPassFailure();
        }

        // Stage 5: Convert QMem to QoalaHost
        LLVM_DEBUG(llvm::dbgs() << "*********************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* 5. Lowering QMem to QoalaHost *\n");
        LLVM_DEBUG(llvm::dbgs() << "*********************************\n");
        LogicalResult qMemToQoalaHostResult =
            mlir::applyPartialConversion(module, qMemToQoalaHostTarget, std::move(qMemToQoalaHostPatterns));
        if (mlir::failed(qMemToQoalaHostResult)) {
            signalPassFailure();
        }

        // Stage 6: Convert QMem to QoalaHost
        LLVM_DEBUG(llvm::dbgs() << "*******************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* 6. Lowering QMem to NetQASM *\n");
        LLVM_DEBUG(llvm::dbgs() << "*******************************\n");
        LogicalResult qMemtoNetQASMResult =
                mlir::applyPartialConversion(module, qMemToNetQASMTarget, std::move(qMemToNetQASMPatterns));
        if (mlir::failed(qMemtoNetQASMResult)) {
            signalPassFailure();
        }
    }
} /* namespace qoala::conversion */
