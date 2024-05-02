#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/Debug.h"

#include "Analysis/Helpers/Helpers.h"
#include "Analysis/QMem/Functionize.h"
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

/* Function copied from the "simple" functionization PoC. Since we will modify how we "group" operations
 * to create the new functions, this function will disappear */
static bool qMemOpCanBeFunctionized(mlir::Operation *op) {
    // A list fo the QMem operation types we would like to "functionize"
    return llvm::isa<
            qmem::CnotOp,
            qmem::CrotXOp,
            qmem::CzOp,
            qmem::EprsMeasureOp,
            qmem::EprsOp,
            qmem::HadamardOp,
            qmem::InitOp,
            qmem::MeasureOp,
            qmem::QAllocOp,
            qmem::RecvFloatsOp,
            qmem::RecvIntsOp,
            qmem::RotateXOp,
            qmem::RotateYOp,
            qmem::RotateZOp,
            qmem::SendFloatsOp,
            qmem::SendIntsOp
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
        ModuleOp operation = dyn_cast<ModuleOp>(getOperation());
        assert(operation);
        LLVM_DEBUG(llvm::dbgs() << "Converting MIR to LIR on module\n");

        // Get a conversion target to define our target dialects
        ConversionTarget target(context);
        target.addLegalDialect<qoalahost::QoalaHostDialect>();
        target.addIllegalDialect<qmem::QMemDialect>();
        // There are NO legal options in QMem dialect after this pass
        //target.addLegalOp<
            // some::Class
        //>();

        // We add the conversion pattern to the context
        RewritePatternSet qMemToQoalaHostPatterns(&context);
        RewritePatternSet qMemToNetQASMPatterns(&context);
        NullTypeConverter typeConverter(&context);
        populateQMemToQoalaHostPatterns(context, qMemToQoalaHostPatterns, typeConverter);
        populateQMemToNetQASMPatterns(context, qMemToNetQASMPatterns, typeConverter);

        // Stage 1: Functionize
        qoala::analysis::functionizeModule(operation, qMemOpCanBeFunctionized);

        // Stage 2: Apply the QMemToQoalaHost conversion patterns
        LogicalResult qMemToQoalaHostResult =
            mlir::applyPartialConversion(operation, target, std::move(qMemToQoalaHostPatterns));
        if (mlir::failed(qMemToQoalaHostResult)) {
            signalPassFailure();
        }

        // Stage 3: Apply the QMemToNetQASM conversion patterns
        LogicalResult qMemToNetQASMResult =
                mlir::applyPartialConversion(operation, target, std::move(qMemToNetQASMPatterns));
        if (mlir::failed(qMemToNetQASMResult)) {
            signalPassFailure();
        }
    }
} /* namespace qoala::conversion */

std::unique_ptr<mlir::Pass> mlir::createQoalaMIRToQoalaLIRPass() {
    return std::make_unique<qoala::conversion::QoalaMIRToQoalaLIRPass>();
}
