#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/Debug.h"

#include "Analysis/Helpers/Helpers.h"
#include "Conversion/Helpers/Helpers.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIRPatterns.h"

#define DEBUG_TYPE "qmem-to-qremote"

using namespace mlir;
using namespace llvm;
using namespace qoala::helpers;
using namespace qoala::dialects;
using namespace qoala::conversion;
using namespace qoala::helpers::angle;

namespace qoala::helpers {
    void configureQMemToQRemoteTarget(ConversionTarget &target) {
        target.addLegalDialect<qremote::QRemoteDialect>();
        target.addIllegalDialect<qmem::QMemDialect>();
        // We declare everything as "legal"
        target.addLegalOp<
#define GET_OP_LIST
#include "Dialect/QMem/QMem.cpp.inc"
        >();
        target.addIllegalOp<
                // Except the remote operation, which is illegal after this pass
                qmem::RemoteOp
        >();
    }

    void populateQMemToQRemotePatterns(
            MLIRContext &context, RewritePatternSet &patterns,
            TypeConverter &typeConverter) {
        patterns.add<
                mir::RemoteOpLowering
        >(typeConverter, &context);
    }
}

namespace qoala::conversion {
#define GEN_PASS_DEF_LOWERQMEMTOQREMOTE
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h.inc"

    class LowerQMemToQRemotePass : public impl::LowerQMemToQRemoteBase<LowerQMemToQRemotePass> {
        using LowerQMemToQRemoteBase::LowerQMemToQRemoteBase;
        void runOnOperation() override;
    };

    void LowerQMemToQRemotePass::runOnOperation() {
        MLIRContext &context = this->getContext();
        ModuleOp module = dyn_cast<ModuleOp>(this->getOperation());
        assert(module);
        LLVM_DEBUG(llvm::dbgs() << "Lowering QMem to QRemote operations on module\n");

        ConversionTarget target(context);
        // We add the conversion pattern to the context
        RewritePatternSet patterns(&context);
        // We don't need a type converter in this stage
        NullTypeConverter typeConverter(&context);

        qoala::helpers::configureQMemToQRemoteTarget(target);
        qoala::helpers::populateQMemToQRemotePatterns(context, patterns, typeConverter);

        LogicalResult result =
                mlir::applyPartialConversion(module, target, std::move(patterns));
        if (mlir::failed(result)) {
            signalPassFailure();
        }
    }
} /* namespace qoala::conversion */