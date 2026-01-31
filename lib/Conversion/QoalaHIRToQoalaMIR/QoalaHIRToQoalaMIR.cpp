#include "llvm/Support/Debug.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlow.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/IR/BuiltinDialect.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"

#include "Analysis/Helpers/PatternRewriteDriver.h"
#include "Conversion/QoalaHIRToQoalaMIR/QoalaHIRToQoalaMIR.h"
#include "Conversion/QoalaHIRToQoalaMIR/QoalaHIRToQoalaMIRPatterns.h"

#define DEBUG_TYPE "QoalaHIRToQoalaMIR"

using namespace mlir;
using namespace llvm;
using namespace qoala::helpers;
using namespace qoala::dialects;

namespace qoala::conversion {
#define GEN_PASS_DEF_QOALAHIRTOQOALAMIR
#include "Conversion/QoalaHIRToQoalaMIR/QoalaHIRToQoalaMIR.h.inc"

    class QoalaHIRToQoalaMIRPass : public impl::QoalaHIRToQoalaMIRBase<QoalaHIRToQoalaMIRPass> {
        using QoalaHIRToQoalaMIRBase::QoalaHIRToQoalaMIRBase;
        void runOnOperation() override;
    };

    void QoalaHIRToQoalaMIRPass::runOnOperation() {
        MLIRContext &context = getContext();
        ModuleOp module = getOperation();
        // Get a conversion target to define our target dialects
        ConversionTarget target(context);
        // We add the legal dialects that we aim to keep in the target
        target.addLegalDialect<qmem::QMemDialect>();
        target.addLegalDialect<BuiltinDialect>();
        target.addLegalDialect<arith::ArithDialect>();
        target.addLegalDialect<math::MathDialect>();
        target.addLegalDialect<tensor::TensorDialect>();
        // Control Flow dialect can exist in MIR, so we declare it legal in the target
        target.addLegalDialect<cf::ControlFlowDialect>();
        // Structured Control Flow can also exist in MIR
        target.addLegalDialect<scf::SCFDialect>();
        // We define the QNet dialect as "illegal", so the conversion will fail
        // if there are any qnet operations in the converted IR
        target.addIllegalDialect<qnet::QNetDialect>();
        // We also declare operations (classes) that can be declared legal in the target
        // dialect.
        // We can also use the `addDynamicallyLegalOp` method to determine at runtime if
        // an operation is legal. The `callback` argument (which receives the operation involved)
        // can determine if it is legal to leave the operation or not.
        /*
        target.addLegalOp<
                //qnet::SomeOp,
                //qnet::SomeOtherOp
        >();
         */
        // For some reason, declaring the UnrealizedConversionCastOp operation as illegal in the
        // target has no effect. (i.e. the IR will be tagged as valid, despite still having some
        // UnrealizedConversionCastOp instances)
        // target.addIllegalOp<UnrealizedConversionCastOp>();

        // We add the conversion pattern to the context
        RewritePatternSet patterns(&context);
        hir::QoalaHIRToQoalaMIRTypeConverter typeConverter(&context);
        patterns.add<hir::FuncOpLowering, hir::ReturnOpLowering, hir::EprsOpLowering, hir::EprsMeasureOpLowering,
                     hir::NewQubitLowering, hir::RemoteOpLowering, hir::RecvIntsOpLowering, hir::RecvFloatsOpLowering,
                     hir::SendIntsOpLowering, hir::SendFloatsOpLowering, hir::RecvIntOpLowering,
                     hir::RecvFloatOpLowering, hir::SendIntOpLowering, hir::SendFloatOpLowering, hir::RotateXLowering,
                     hir::RotateYLowering, hir::RotateZLowering, hir::HadamardLowering, hir::XLowering, hir::YLowering,
                     hir::ZLowering, hir::CNotLowering, hir::CzLowering, hir::CRotXLowering, hir::MeasureLowering,
                     hir::RotateXIntLowering, hir::RotateYIntLowering, hir::RotateZIntLowering>(
                typeConverter, &context);

        // We apply a **full** conversion, since we correctly defined all the
        // dialects that are "legal" in the target IR
        if (failed(applyFullConversion(module.getOperation(), target, std::move(patterns)))) {
            LLVM_DEBUG(llvm::dbgs() << module << "\n");
            signalPassFailure();
        }

        // Specific conversion target and pattern set for "lowering" scf.if
        ConversionTarget scfTarget(getContext());
        RewritePatternSet scfPatterns(&context);
        // We need to do this since the scf "lowering" needs to happen *after* the main lowering.
        // This is due to the fact that we need to use a few tricks to process the scf.if:
        // * We mark the whole SCF dialect is legal
        // * We mark *only* the scf.if op as illegal (this allows triggering the lowering pattern)
        // * The lowering pattern creates a *new* scf.if op, with the needed modifications
        // * The old scf.if is replaced with the modified one.
        // If we don't separate this lowering target (and put them in the lowering target above)
        // then we introduce an infinite loop in the lowering process.

        // Declare the SCF target as legal
        scfTarget.addLegalDialect<scf::SCFDialect>();
        // however, since we need to trigger the "lowering" of the scf.if operations, we mark *only*
        // that operation as illegal
        scfTarget.addIllegalOp<scf::IfOp>();
        scfPatterns.add<hir::ScfIfRewriting>(&context);

        port::GreedyRewriteConfig cfg;
        // If we don't disable region simplification, the folding of the code will be more aggressive
        // simplifying entire blocks if possible:
        // E.g, in test/Conversion/MIRtoLIR/BlkMeta/MIRtoLIR-precedences-predecessors.mlir
        // the aggressive folding will merge both "then" and "else" blocks into a single one,
        // inserting a block argument. That makes the test fail, since it expects 4 blocks, not 3.
        cfg.setRegionSimplificationLevel(port::GreedySimplifyRegionLevel::Disabled);
        // Since we use the ported pattern rewriter driver, we can disable code folding and constant CSE
        cfg.enableFolding(false);
        cfg.enableConstantCSE(false);
        cfg.setStrictness(port::GreedyRewriteStrictness::ExistingOps);

        // Apply the rewrite pattern, using the ported driver, which can turn off
        // CSE folding and region simplification.
        if (failed(port::applyPatternsGreedily(module.getOperation(), std::move(scfPatterns), cfg))) {
            LLVM_DEBUG(llvm::dbgs() << module << "\n");
            signalPassFailure();
        }

        // Finally, we have to get rid of any leftover UnrealizedConversionCastOp... provided that
        // they don't have any uses. If we find one that has one or more uses, then the dialect
        // conversion process is bugged.
        // NB: This is an ugly, yet effective solution.
        WalkResult result = module.walk([](const UnrealizedConversionCastOp leftoverCast) -> WalkResult {
            if (leftoverCast->getUses().empty()) {
                leftoverCast->erase();
                return WalkResult::advance();
            }
            leftoverCast->emitOpError("Leftover unrealized cast still has usages. ")
                    << "This is a bug on the implementation of the HIR to MIR lowering.";
            return WalkResult::interrupt();
        });

        if (result.wasInterrupted()) {
            signalPassFailure();
        }
    }
} /* namespace qoala::conversion */
