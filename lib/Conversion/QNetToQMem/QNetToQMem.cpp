#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"

#include "Conversion/Helpers/Helpers.h"
#include "Conversion/QNetToQMem/QNetToQMem.h"
#include "Conversion/QNetToQMem/QNetToQMemPatterns.h"

namespace mlir {
#define GEN_PASS_DEF_QNETTOQMEM
#include "Conversion/QNetToQMem/QNetToQMem.h.inc"
} // namespace mlir

using namespace mlir;
using namespace llvm;
using namespace qoala::helpers;
using namespace qoala::dialects;

namespace qoala::conversion {
class QNetToQMemPass : public mlir::impl::QNetToQMemBase<QNetToQMemPass> {
    void runOnOperation() override;
};
} /* namespace qoala::conversion */

void qoala::conversion::QNetToQMemPass::runOnOperation() {
    MLIRContext &context = getContext();
    Operation *operation = getOperation();
    // Get a conversion target to define our target dialects
    ConversionTarget target(context);
    // We add the legal dialects that we aim to keep in the target
    target.addLegalDialect<lir::LirDialect>();
    // We define the QNet dialect as "illegal", so the conversion will fail
    // if there are any qnet operations in the converted IR
    target.addIllegalDialect<qnet::QNetDialect>();
    // We also declare operations that can be declared legal in the target
    // dialect. We callback argument (which receives the operation involved) can
    // determine if
    // it is legal to leave the operation or not.
    target.addDynamicallyLegalOp<qnet::RotXOp>(
        [](qnet::RotXOp op) { return true; });
    target.addDynamicallyLegalOp<qnet::RotYOp>(
        [](qnet::RotYOp op) { return true; });
    target.addDynamicallyLegalOp<qnet::CnotOp>(
        [](qnet::CnotOp op) { return true; });

    // We add the conversion pattern to the context
    RewritePatternSet patterns(&context);
    QNetToQMemQubitTypeConverter typeConverter(&context);
    patterns.add<NewQubitOpLowering, MeasureQubitOpLowering, RotZOpLowering>(
        typeConverter, &context);

    // We finally apply a **partial** conversion, since there will be some
    // operations that will stay... momentarily
    LogicalResult result =
        mlir::applyPartialConversion(operation, target, std::move(patterns));
    if (mlir::failed(result)) {
        signalPassFailure();
    }
}

std::unique_ptr<mlir::Pass> mlir::createQNetToQMemPass() {
    return std::make_unique<qoala::conversion::QNetToQMemPass>();
}
