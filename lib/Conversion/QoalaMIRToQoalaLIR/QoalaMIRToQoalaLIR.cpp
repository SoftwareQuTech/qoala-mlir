#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"

#include "Conversion/Helpers/Helpers.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIRPatterns.h"

namespace mlir {
#define GEN_PASS_DEF_QOALAMIRTOQOALALIR
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h.inc"
} // namespace mlir

#include "llvm/Support/Debug.h"

using namespace mlir;
using namespace llvm;
using namespace qoala::helpers;
using namespace qoala::dialects;

namespace qoala::conversion {
    class QoalaMIRToQoalaLIRPass : public mlir::impl::QoalaMIRToQoalaLIRBase<QoalaMIRToQoalaLIRPass> {
        void runOnOperation() override;
    };

    void QoalaMIRToQoalaLIRPass::runOnOperation() {
        MLIRContext &context = getContext();
        ModuleOp operation = dyn_cast<ModuleOp>(getOperation());

        // Get a conversion target to define our target dialects
        ConversionTarget target(context);
        // We add the legal dialects that we aim to keep in the target
        target.addLegalDialect<qoalahost::QoalaHostDialect>();
        // We define the QNet dialect as "illegal", so the conversion will fail
        // if there are any qnet operations in the converted IR
        target.addIllegalDialect<qmem::QMemDialect>();
        // We also declare operations (classes) that can be declared legal in the target
        // dialect. The `callback` argument (which receives the operation involved)
        // can determine if it is legal to leave the operation or not.
        target.addLegalOp<
#define GET_OP_LIST
#include "Dialect/QMem/QMem.cpp.inc"
        >();

        // We add the conversion pattern to the context
        RewritePatternSet patterns(&context);
        QoalaMIRToQoalaLIRTypeConverter typeConverter(&context);
//        patterns.add<
//                //TODO
//        >(typeConverter, &context);

        // We finally apply a **partial** conversion, since there will be some
        // operations that will stay... momentarily
        LogicalResult result =
            mlir::applyPartialConversion(operation, target, std::move(patterns));
        if (mlir::failed(result)) {
            signalPassFailure();
        }
    }
} /* namespace qoala::conversion */

std::unique_ptr<mlir::Pass> mlir::createQoalaMIRToQoalaLIRPass() {
    return std::make_unique<qoala::conversion::QoalaMIRToQoalaLIRPass>();
}

std::unique_ptr<mlir::Pass> mlir::createLowerQMemToQoalaHostPass() {
    return nullptr;
}

std::unique_ptr<mlir::Pass> mlir::createLowerQMemToNetQASMPass() {
    return nullptr;
}
