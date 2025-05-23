#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Transforms/Passes.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/Debug.h"

#include "Analysis/Helpers/Helpers.h"
#include "Analysis/QMem/Conversion.h"
#include "Analysis/QoalaHost/Helpers.h"
#include "Conversion/Helpers/Helpers.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h"

#define DEBUG_TYPE "mir-to-lir"

using namespace mlir;
using namespace qoala::dialects;

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
        ModuleOp module = this->getOperation();
        LLVM_DEBUG(llvm::dbgs() << "*************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* Converting MIR to LIR *\n");
        LLVM_DEBUG(llvm::dbgs() << "*************************\n");

        PassManager passManager = PassManager::on<ModuleOp>(&context);

        helpers::NullTypeConverter typeConverter(&context);

        // Configuration of the conversion targets and patterns for each stage
        ConversionTarget f32LoweringTarget(context);
        RewritePatternSet f32Patterns(&context);
        helpers::configureF32LoweringTarget(f32LoweringTarget);
        helpers::populateQMemF32ToInt32RotPatterns(context, f32Patterns, typeConverter);

        ConversionTarget qMemToQoalaHostTarget(context);
        RewritePatternSet qMemToQoalaHostPatterns(&context);
        helpers::configureQMemToQoalaHostTarget(qMemToQoalaHostTarget, true, false);
        helpers::populateQMemToQoalaHostPatterns(context, qMemToQoalaHostPatterns, typeConverter);

        ConversionTarget qMemToNetQASMTarget(context);
        RewritePatternSet qMemToNetQASMPatterns(&context);
        helpers::configureQMemToNetQASMTarget(qMemToNetQASMTarget);
        helpers::populateQMemToNetQASMPatterns(context, qMemToNetQASMPatterns, typeConverter);

        ConversionTarget qMemToQRemoteTarget(context);
        RewritePatternSet qMemToQRemotePatterns(&context);
        helpers::configureQMemToQRemoteTarget(qMemToQRemoteTarget);
        helpers::populateQMemToQRemotePatterns(context, qMemToQRemotePatterns, typeConverter);

        // TODO - Add lowering for Affine/SCF -> CF, Tensor -> Memref, Async

        // Stage 1: Insert the declaration of the builtin angle conversion function
        LLVM_DEBUG(llvm::dbgs() << "*********************************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* 1. Inserting builtin function declaration *\n");
        LLVM_DEBUG(llvm::dbgs() << "*********************************************\n");
        // Note: This is the way how we will handle "dynamic" f32 values in the future.
        // We will keep inserting this declaration and assuming it will be provided by the runtime in the future.
        if (!helpers::angle::moduleContainsAngleConversionDeclaration(module)) {
            helpers::angle::insertAngleConversionFunctionDeclaration(module);
        }

        if (this->useSCCP) {
            // We use this code switch to create an SCCP (Sparse Conditional Constant Propagation) pass between stages 1 and 2 to propagate
            // constants inside branching blocks. This will be needed when supporting branching
            // instructions inside the body of a NetQASM local routine (for example, when performing
            // a different rotation based on the value of an argument).
            // We create an SCCP pass to try to
            // propagate the constants inside the basic blocks of the  main function
            // We need this to try to compile the rotations and move them into local routines
            // when used in conjunction with branching instructions.
            LLVM_DEBUG(llvm::dbgs() << "*********************************************\n");
            LLVM_DEBUG(llvm::dbgs() << "*** Before SCCP:\n");
            LLVM_DEBUG(llvm::dbgs() << module << "\n");
            LLVM_DEBUG(llvm::dbgs() << "*********************************************\n");

            auto mainFuncs = module.getOps<qmem::FuncOp>();
            assert (!mainFuncs.empty());
            qmem::FuncOp mainFunc = *mainFuncs.begin();

            auto pm = PassManager::on<qmem::FuncOp>(&context);
            pm.addPass(createSCCPPass());
            if (failed(pm.run(mainFunc))) {
                signalPassFailure();
            }

            LLVM_DEBUG(llvm::dbgs() << "*********************************************\n");
            LLVM_DEBUG(llvm::dbgs() << "*** After SCCP:\n");
            LLVM_DEBUG(llvm::dbgs() << module << "\n");
            LLVM_DEBUG(llvm::dbgs() << "*********************************************\n");
        }

        // Stage 2: Try to fold operations as much as possible, especially, constants
        LLVM_DEBUG(llvm::dbgs() << "************************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* 2. Folding (constant) operations *\n");
        LLVM_DEBUG(llvm::dbgs() << "************************************\n");
        if (failed(helpers::foldConstants(module))) {
            signalPassFailure();
        }

        // Stage 3: Transform f32 operations to their i32 counterparts - This is done with an "intra-dialect" lowering
        LLVM_DEBUG(llvm::dbgs() << "*****************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* 3. Lowering f32 rotations *\n");
        LLVM_DEBUG(llvm::dbgs() << "*****************************\n");
        if (failed(applyPartialConversion(module, f32LoweringTarget, std::move(f32Patterns)))) {
            signalPassFailure();
        }

        // Stage 4: After compiling f32 rotations, some constants could now be orphan operations; remove them.
        LLVM_DEBUG(llvm::dbgs() << "*****************************************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* 4. Removing unnecessary constants (Folding again) *\n");
        LLVM_DEBUG(llvm::dbgs() << "*****************************************************\n");

        if (failed(helpers::foldConstants(module))) {
            signalPassFailure();
        }

        // Stage 5: Functionize
        LLVM_DEBUG(llvm::dbgs() << "***************************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* 5. Functionizing quantum operations *\n");
        LLVM_DEBUG(llvm::dbgs() << "***************************************\n");

        if (this->useSimpleFunctionize) {
            LLVM_DEBUG(llvm::dbgs() << "WARNING - Using simple functionization\n");
            analysis::functionize::functionizeModule(module, analysis::functionize::simpleOpClassifier, this->maxOpsPerGroup);
        } else {
            analysis::functionize::functionizeModule(module, analysis::functionize::functionizeOpClassifier, this->maxOpsPerGroup);
        }
        // Correct the positions of the remote and builtin declaration
        module.walk([&](func::FuncOp funcDecl) {
            if (funcDecl.getSymName() != helpers::angle::angleConversionFunctionName) {
                WalkResult::advance();
            } else {
                helpers::moveOperationToTop(module, funcDecl);
            }
        });
        module.walk([&](const qmem::RemoteOp remote) {
            helpers::moveOperationToTop(module, remote);
        });

        // Stage 6: Transform f32 operations to their i32 counterparts - This is done with an "intra-dialect" lowering
        LLVM_DEBUG(llvm::dbgs() << "***********************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* 6. Lowering Remote declarations *\n");
        LLVM_DEBUG(llvm::dbgs() << "***********************************\n");
        if (failed(applyPartialConversion(module, qMemToQRemoteTarget, std::move(qMemToQRemotePatterns)))) {
            signalPassFailure();
        }

        // Stage 7: Convert QMem to QoalaHost
        LLVM_DEBUG(llvm::dbgs() << "*********************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* 7. Lowering QMem to QoalaHost *\n");
        LLVM_DEBUG(llvm::dbgs() << "*********************************\n");
        if (failed(applyPartialConversion(module, qMemToQoalaHostTarget, std::move(qMemToQoalaHostPatterns)))) {
            signalPassFailure();
        }

        // Stage 8: Convert QMem to QoalaHost
        LLVM_DEBUG(llvm::dbgs() << "*******************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* 8. Lowering QMem to NetQASM *\n");
        LLVM_DEBUG(llvm::dbgs() << "*******************************\n");
        if (failed(applyPartialConversion(module, qMemToNetQASMTarget, std::move(qMemToNetQASMPatterns)))) {
            signalPassFailure();
        }
        // Stage 9: Add Block Precedences
        LLVM_DEBUG(llvm::dbgs() << "********************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* 9. Adding Block Precedences *\n");
        LLVM_DEBUG(llvm::dbgs() << "********************************\n");
        if (failed(analysis::precedences::addPrecedences(module))) {
            signalPassFailure();
        }
    }
} /* namespace qoala::conversion */
