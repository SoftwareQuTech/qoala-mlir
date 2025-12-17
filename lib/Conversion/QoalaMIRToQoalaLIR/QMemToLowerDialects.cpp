#include "llvm/Support/Debug.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"

#include "Analysis/Helpers/Helpers.h"
#include "Analysis/QoalaHost/Helpers.h"
#include "Analysis/QoalaHost/RemoteIDs.h"
#include "Conversion/Helpers/Helpers.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIRPatterns.h"

#define DEBUG_TYPE "qmem-to-qoalahost"

using namespace mlir;
using namespace llvm;
using namespace qoala::helpers;
using namespace qoala::dialects;
using namespace qoala::conversion;
using namespace qoala::helpers::angle;

namespace qoala::helpers {
    void configureQMemToQoalaHostTarget(ConversionTarget &target, const bool intRotsAreLegal,
                                        const bool floatRotsAreLegal) {
        target.addLegalDialect<qoalahost::QoalaHostDialect>();
        target.addIllegalDialect<qmem::QMemDialect>();
        target.addLegalOp<
                // We declare as "legal" all the operations that directly map to NetQASM operations
                qmem::CnotOp, qmem::CzOp, qmem::EprsMeasureOp, qmem::EprsOp, qmem::HadamardOp, qmem::InitOp,
                qmem::MeasureOp, qmem::QAllocOp>();
        if (intRotsAreLegal) {
            target.addLegalOp<qmem::RotateXIntOp, qmem::RotateYIntOp, qmem::RotateZIntOp, qmem::CrotXIntOp>();
        } else {
            target.addIllegalOp<qmem::RotateXIntOp, qmem::RotateYIntOp, qmem::RotateZIntOp, qmem::CrotXIntOp>();
        }
        if (floatRotsAreLegal) {
            target.addLegalOp<qmem::RotateXOp, qmem::RotateYOp, qmem::RotateZOp, qmem::CrotXOp>();
        } else {
            target.addIllegalOp<qmem::RotateXOp, qmem::RotateYOp, qmem::RotateZOp, qmem::CrotXOp>();
        }
    }

    void populateQMemToQoalaHostPatterns(MLIRContext &context, RewritePatternSet &patterns,
                                         TypeConverter &typeConverter) {
        patterns.add<mir::RemoteOpLowering, mir::FuncOpLowering, mir::ReturnOpLowering, mir::CallOpLowering>(
                typeConverter, &context);
        patterns.add<mir::RecvIntOpLowering, mir::RecvFloatOpLowering, mir::SendIntOpLowering,
                     mir::SendFloatOpLowering>(typeConverter, &context);
        patterns.add<mir::RecvIntsOpLowering, mir::RecvFloatsOpLowering, mir::SendIntsOpLowering,
                     mir::SendFloatsOpLowering>(typeConverter, &context);
    }

    void configureQMemToNetQASMTarget(ConversionTarget &target) {
        target.addLegalDialect<netqasm::NetQASMDialect>();
        target.addIllegalDialect<qmem::QMemDialect>();
        target.addLegalOp<
                // We declare as "legal" all the operations that directly map to QoalaHost operations
                qmem::FuncOp, qmem::ReturnOp, qmem::RemoteOp, qmem::RecvIntsOp, qmem::RecvFloatsOp, qmem::SendIntsOp,
                qmem::SendFloatsOp>();
    }

    void populateQMemToNetQASMPatterns(MLIRContext &context, RewritePatternSet &patterns,
                                       TypeConverter &typeConverter) {
        patterns.add<mir::MeasureOpLowering, mir::EprsOpLowering, mir::EprsMeasureOpLowering,
                     mir::NetQASMFunctionLowering, mir::NetQASMReturnOpLowering, mir::QAllocLowering,
                     mir::QInitLowering, mir::HadamardLowering, mir::CNotLowering, mir::CzLowering,
                     mir::RotateXIntLowering, mir::RotateYIntLowering, mir::RotateZIntLowering, mir::CRotXIntLowering>(
                typeConverter, &context);
    }

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
                qmem::RemoteOp>();
    }

    void populateQMemToQRemotePatterns(MLIRContext &context, RewritePatternSet &patterns,
                                       TypeConverter &typeConverter) {
        patterns.add<mir::RemoteOpLowering>(typeConverter, &context);
    }
} // namespace qoala::helpers

namespace qoala::conversion {
#define GEN_PASS_DEF_LOWERQMEMTOLOWERDIALECTS
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h.inc"

    class LowerQMemToLowerDialectsPass : public impl::LowerQMemToLowerDialectsBase<LowerQMemToLowerDialectsPass> {
        using LowerQMemToLowerDialectsBase::LowerQMemToLowerDialectsBase;
        void runOnOperation() override;
    };

    void LowerQMemToLowerDialectsPass::runOnOperation() {
        MLIRContext &context = this->getContext();
        ModuleOp module = this->getOperation();
        NullTypeConverter typeConverter(&context);

        ConversionTarget qMemToQoalaHostTarget(context);
        RewritePatternSet qMemToQoalaHostPatterns(&context);
        configureQMemToQoalaHostTarget(qMemToQoalaHostTarget, true, false);
        populateQMemToQoalaHostPatterns(context, qMemToQoalaHostPatterns, typeConverter);

        ConversionTarget qMemToNetQASMTarget(context);
        RewritePatternSet qMemToNetQASMPatterns(&context);
        configureQMemToNetQASMTarget(qMemToNetQASMTarget);
        populateQMemToNetQASMPatterns(context, qMemToNetQASMPatterns, typeConverter);

        ConversionTarget qMemToQRemoteTarget(context);
        RewritePatternSet qMemToQRemotePatterns(&context);
        configureQMemToQRemoteTarget(qMemToQRemoteTarget);
        populateQMemToQRemotePatterns(context, qMemToQRemotePatterns, typeConverter);

        // Stage 1: Lower the remote declarations
        LLVM_DEBUG(llvm::dbgs() << "********************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* Lowering Remote declarations *\n");
        LLVM_DEBUG(llvm::dbgs() << "********************************\n");
        if (failed(applyPartialConversion(module, qMemToQRemoteTarget, std::move(qMemToQRemotePatterns)))) {
            signalPassFailure();
        }

        // Stage 2: Convert QMem to QoalaHost
        LLVM_DEBUG(llvm::dbgs() << "******************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* Lowering QMem to QoalaHost *\n");
        LLVM_DEBUG(llvm::dbgs() << "******************************\n");
        if (failed(applyPartialConversion(module, qMemToQoalaHostTarget, std::move(qMemToQoalaHostPatterns)))) {
            signalPassFailure();
        }

        // Stage 3: Convert QMem to QoalaHost
        LLVM_DEBUG(llvm::dbgs() << "****************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* Lowering QMem to NetQASM *\n");
        LLVM_DEBUG(llvm::dbgs() << "****************************\n");
        if (failed(applyPartialConversion(module, qMemToNetQASMTarget, std::move(qMemToNetQASMPatterns)))) {
            signalPassFailure();
        }

        // Stage 4: Move Entanglement Blocks at the beginning
        LLVM_DEBUG(llvm::dbgs() << "*********************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* Moving Entanglement Blocks *\n");
        LLVM_DEBUG(llvm::dbgs() << "*********************************\n");
        if (options::qoalaOptGroupEntReqs) {
            analysis::reordering::groupEntanglementBlocksFirst(module);
        }

        // Stage 5: Add Block Precedences
        LLVM_DEBUG(llvm::dbgs() << "****************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* Adding Block Precedences *\n");
        LLVM_DEBUG(llvm::dbgs() << "****************************\n");
        if (failed(analysis::precedences::addPrecedences(module))) {
            signalPassFailure();
        }

        // Stage 6: Insert socket IDs placeholders.
        LLVM_DEBUG(llvm::dbgs() << "**********************************\n");
        LLVM_DEBUG(llvm::dbgs() << "* Adding Socket IDs placeholders *\n");
        LLVM_DEBUG(llvm::dbgs() << "**********************************\n");
        if (failed(analysis::remoteids::addRemoteIDs(module))) {
            signalPassFailure();
        }
    }
} /* namespace qoala::conversion */
