#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/Debug.h"

#include "Conversion/Helpers/Helpers.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h"

namespace mlir {
#define GEN_PASS_DEF_LOWERQMEMTONETQASM
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h.inc"
} // namespace mlir

#define DEBUG_TYPE "qmem-to-netqasm"

using namespace mlir;
using namespace llvm;
using namespace qoala::helpers;
using namespace qoala::dialects;

namespace qoala::conversion {
    class LowerQMemToNetQASMPass : public mlir::impl::LowerQMemToNetQASMBase<LowerQMemToNetQASMPass> {
        void runOnOperation() override;
    };

    void LowerQMemToNetQASMPass::runOnOperation() {
        MLIRContext &context = this->getContext();
        ModuleOp operation = dyn_cast<ModuleOp>(getOperation());

        ConversionTarget target(context);
        target.addLegalDialect<netqasm::NetQASMDialect>();
        target.addIllegalDialect<qmem::QMemDialect>();
        target.addLegalOp<
                // Only the "qmem.func" and "qmem.return" operations are legal after this pass,
                // because this pass modifies ALL the other operations but the main function
                qmem::FuncOp,
                qmem::ReturnOp
        >();

        // We add the conversion pattern to the context
        RewritePatternSet patterns(&context);
        // We don't need a type converter in this stage
        //QoalaMIRToQoalaLIRTypeConverter typeConverter(&context);
//        patterns.add<
                // TODO - To Implement
//        >(&context);

        LogicalResult result =
                mlir::applyPartialConversion(operation, target, std::move(patterns));
        if (mlir::failed(result)) {
            signalPassFailure();
        }
    }
}

std::unique_ptr<mlir::Pass> mlir::createLowerQMemToNetQASMPass() {
    return std::make_unique<qoala::conversion::LowerQMemToNetQASMPass>();
}
