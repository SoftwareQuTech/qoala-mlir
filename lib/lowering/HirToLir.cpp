#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Pass/Pass.h"

#include "lowering/HIRToLIR.h"
#include "lowering/Helpers.h"
#include "lowering/HIRToLIRPatterns.h"

namespace mlir {
#define GEN_PASS_DEF_HIRTOLIR
#include "lowering/HIRToLIR.h.inc"
}

using namespace mlir;
using namespace llvm;
using namespace qoala::helpers;
using namespace qoala::lowering;


class HIRtoLIRPass : public mlir::impl::HIRtoLIRBase<HIRtoLIRPass> {
    void runOnOperation() override;
};

void HIRtoLIRPass::runOnOperation() {
    MLIRContext &context = getContext();
    Operation *operation = getOperation();
    // Get a conversion target to define our target dialects
    ConversionTarget target(context);
    // We add the legal dialects that we aim to keep in the target
    target.addLegalDialect<lir::LirDialect>();
    // We define the hir dialect as "illegal", so the conversion will fail
    // if there are any hir operations in the converted IR
    target.addIllegalDialect<hir::HirDialect>();
    // We also declare operations that can be declared legal in the target dialect.
    // We callback argument (which receives the operation involved) can determine if
    //it is legal to leave the operation or not.
    target.addDynamicallyLegalOp<hir::RotXOp>([](hir::RotXOp op) {
        return true;
    });
    target.addDynamicallyLegalOp<hir::RotYOp>([](hir::RotYOp op) {
        return true;
    });
    target.addDynamicallyLegalOp<hir::CnotOp>([](hir::CnotOp op) {
        return true;
    });

    // We add the conversion pattern to the context
    RewritePatternSet patterns(&context);
    HirQubitToLirQubitTypeConverter typeConverter(&context);
    patterns.add<
            NewQubitOpLowering,
            MeasureQubitOpLowering,
            RotZOpLowering
            >(typeConverter, &context);


    // We finally apply a **partial** conversion, since there will be some operations that will stay... momentarily
    LogicalResult result = mlir::applyPartialConversion(operation, target, std::move(patterns));
    if (mlir::failed(result)) {
        signalPassFailure();
    }
}

std::unique_ptr<mlir::Pass> mlir::createHirToLirPass() {
    return std::make_unique<HIRtoLIRPass>();
}
