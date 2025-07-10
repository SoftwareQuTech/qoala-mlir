#include "llvm/Support/Debug.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Transforms/Passes.h"

#include "Analysis/Helpers/Helpers.h"
#include "Conversion/Helpers/Helpers.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h"

namespace qoala::analysis {
#define GEN_PASS_DECL
#include "Dialect/Helpers/HelperPasses.h.inc"
} // namespace qoala::analysis

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

        OpPassManager passManager("builtin.module");

        helpers::NullTypeConverter typeConverter(&context);

        // Configuration of the conversion targets and patterns for each stage
        ConversionTarget f32LoweringTarget(context);
        RewritePatternSet f32Patterns(&context);
        helpers::configureF32LoweringTarget(f32LoweringTarget);
        helpers::populateQMemF32ToInt32RotPatterns(context, f32Patterns, typeConverter);

        // TODO - Add lowering for Affine/SCF -> CF, Tensor -> Memref, Async

        // Stage 1: Insert the declaration of the builtin angle conversion function
        // LLVM_DEBUG(llvm::dbgs() << "*********************************************\n");
        // LLVM_DEBUG(llvm::dbgs() << "* 1. Inserting builtin function declaration *\n");
        // LLVM_DEBUG(llvm::dbgs() << "*********************************************\n");
        // Note: This is the way how we will handle "dynamic" f32 values in the future.
        // We will keep inserting this declaration and assuming it will be provided by the runtime in the future.
        if (!helpers::angle::moduleContainsAngleConversionDeclaration(module)) {
            // helpers::angle::insertAngleConversionFunctionDeclaration(module);
            passManager.addPass(analysis::createAngleConversionDeclaration());
        }

        OpPassManager &funcOpPassManager = passManager.nest<qmem::FuncOp>();
        if (this->useSCCP) {
            // We use this code switch to create an SCCP (Sparse Conditional Constant Propagation) pass between stages 1
            // and 2 to propagate constants inside branching blocks. This will be needed when supporting branching
            // instructions inside the body of a NetQASM local routine (for example, when performing
            // a different rotation based on the value of an argument).
            // We create an SCCP pass to try to
            // propagate the constants inside the basic blocks of the  main function
            // We need this to try to compile the rotations and move them into local routines
            // when used in conjunction with branching instructions.

            // LLVM_DEBUG(llvm::dbgs() << "*********************************************\n");
            // LLVM_DEBUG(llvm::dbgs() << "*** Before SCCP:\n");
            // LLVM_DEBUG(llvm::dbgs() << module << "\n");
            // LLVM_DEBUG(llvm::dbgs() << "*********************************************\n");

            // auto mainFuncs = module.getOps<qmem::FuncOp>();
            // assert (!mainFuncs.empty());
            // qmem::FuncOp mainFunc = *mainFuncs.begin();

            funcOpPassManager.addPass(createSCCPPass());
            // if (failed(pm.run(mainFunc))) {
            //     signalPassFailure();
            // }

            // LLVM_DEBUG(llvm::dbgs() << "*********************************************\n");
            // LLVM_DEBUG(llvm::dbgs() << "*** After SCCP:\n");
            // LLVM_DEBUG(llvm::dbgs() << module << "\n");
            // LLVM_DEBUG(llvm::dbgs() << "*********************************************\n");
        }

        // Stage 2: Try to fold operations as much as possible, especially, constants
        // LLVM_DEBUG(llvm::dbgs() << "************************************\n");
        // LLVM_DEBUG(llvm::dbgs() << "* 2. Folding (constant) operations *\n");
        // LLVM_DEBUG(llvm::dbgs() << "************************************\n");
        // if (failed(helpers::foldConstants(module))) {
        //     signalPassFailure();
        // }
        funcOpPassManager.addPass(analysis::createFoldConstants());

        // Stage 3: Transform f32 operations to their i32 counterparts - This is done with an "intra-dialect" lowering
        // LLVM_DEBUG(llvm::dbgs() << "*****************************\n");
        // LLVM_DEBUG(llvm::dbgs() << "* 3. Lowering f32 rotations *\n");
        // LLVM_DEBUG(llvm::dbgs() << "*****************************\n");
        // if (failed(applyPartialConversion(module, f32LoweringTarget, std::move(f32Patterns)))) {
        //     signalPassFailure();
        // }
        funcOpPassManager.addPass(analysis::createConvertIntegerToFloatRotations());

        // Stage 4: After compiling f32 rotations, some constants could now be orphan operations; remove them.
        // LLVM_DEBUG(llvm::dbgs() << "*****************************************************\n");
        // LLVM_DEBUG(llvm::dbgs() << "* 4. Removing unnecessary constants (Folding again) *\n");
        // LLVM_DEBUG(llvm::dbgs() << "*****************************************************\n");
        //
        // if (failed(helpers::foldConstants(module))) {
        //     signalPassFailure();
        // }
        passManager.addPass(analysis::createFoldConstants());

        // Stage 5: Functionize
        // LLVM_DEBUG(llvm::dbgs() << "***************************************\n");
        // LLVM_DEBUG(llvm::dbgs() << "* 5. Functionizing quantum operations *\n");
        // LLVM_DEBUG(llvm::dbgs() << "***************************************\n");

        // if (this->useSimpleFunctionize) {
        //     LLVM_DEBUG(llvm::dbgs() << "WARNING - Using simple functionization\n");
        //     analysis::functionize::functionizeModule(module, analysis::functionize::simpleOpClassifier,
        //                                              this->maxOpsPerGroup);
        // } else {
        //     analysis::functionize::functionizeModule(module, analysis::functionize::functionizeOpClassifier,
        //                                              this->maxOpsPerGroup);
        // }
        // Correct the positions of the remote and builtin declaration
        // module.walk([&](func::FuncOp funcDecl) {
        //     if (funcDecl.getSymName() != helpers::angle::angleConversionFunctionName) {
        //         WalkResult::advance();
        //     } else {
        //         helpers::moveOperationToTop(module, funcDecl);
        //     }
        // });
        // module.walk([&](const qmem::RemoteOp remote) { helpers::moveOperationToTop(module, remote); });

        passManager.addPass(analysis::createFunctionizeQuantumOps({this->useSimpleFunctionize, this->maxOpsPerGroup}));

        // This is the limit of what can be isolated in standalone passes.
        // Converting QMem to NetQASM and QoalaHost, and then adding blk_meta operations
        // *NEEDS* to be done in a single pass, since between these transformations the IR
        // is in an invalid state.
        // According to the MLIR programmer manual, each pass *must* leave valid IR
        // after running:
        // https://mlir.llvm.org/getting_started/DeveloperGuide/#ir-should-be-valid-before-and-after-each-pass
        // For this reason, we group app the transformations in a single pass
        passManager.addPass(createLowerQMemToLowerDialects());

        LLVM_DEBUG(llvm::dbgs() << "pass pipeline:\n");
        passManager.printAsTextualPipeline(llvm::dbgs());
        LLVM_DEBUG(llvm::dbgs() << "\n*******************************************\n");

        if (failed(runPipeline(passManager, module))) {
            signalPassFailure();
        }
    }
} /* namespace qoala::conversion */
