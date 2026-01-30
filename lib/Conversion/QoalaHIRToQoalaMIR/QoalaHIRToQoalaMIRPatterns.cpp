#include "Conversion/QoalaHIRToQoalaMIR/QoalaHIRToQoalaMIRPatterns.h"
#include "Conversion/Helpers/Helpers.h"
#include "Dialect/QNet/QNetDialect.h"

#include "llvm/ADT/TypeSwitch.h"

#define DEBUG_TYPE "qoala-hir-to-qoala-patterns"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::helpers;

namespace qoala::conversion::hir {
    /* Implementation of the qoala types converter */
    QoalaHIRToQoalaMIRTypeConverter::QoalaHIRToQoalaMIRTypeConverter(MLIRContext *ctx) {
        // Default conversion for non qnet::QubitType instances
        addConversion([](const Type type) -> Type { return type; });
        addConversion([ctx](qnet::QubitType type) -> Type {
            // Qubit Types are mapped into i32 (pointers)
            return IntegerType::get(ctx, 32);
        });
        // In case there will be illegal values (values of types declared illegal)
        // after conversion, then we need to provide a "materialization", i.e. how
        // to cast from one type to the other Here we provide a "cast" to transform
        // from source type (qnet::QubitType) to target type (qmem::QubitType = i32)
        addTargetMaterialization(
                [](OpBuilder &builder, Type resultType, ValueRange inputs, const Location loc) -> Value {
                    // The conversion is simply a "builtin::unrealized_cast" operation
                    return builder.create<UnrealizedConversionCastOp>(loc, resultType, inputs).getResult(0);
                });
        // Here we provide a "cast" to transform from target type (lir::QubitType)
        // to source type (qnet::QubitType)
        addSourceMaterialization(
                [](OpBuilder &builder, Type resultType, ValueRange inputs, const Location loc) -> std::optional<Value> {
                    // The conversion is simply a "builtin::unrealized_cast" operation
                    return builder.create<UnrealizedConversionCastOp>(loc, resultType, inputs).getResult(0);
                });
    }

    std::unique_ptr<OpAndValues> FuncOpLowering::createNewOpAndValues(qnet::FuncOp op, qnet::FuncOp::Adaptor adaptor,
                                                                      ConversionPatternRewriter &rewriter) const {
        auto newFunc = rewriter.create<qmem::FuncOp>(op.getLoc(), adaptor.getSymName(), adaptor.getFunctionType(),
                                                     adaptor.getAttributes().getValue(), ArrayRef<DictionaryAttr>{});
        // Before returning the new function, we need to "attach" (inline)
        // the body (region) of the old operation to the new one
        // This is inspired in the last part of the `convertFuncOpToLLVMFuncOp`
        // function in llvm/mlir/lib/Conversion/FuncToLLVM/FuncToLLVM.cpp
        rewriter.inlineRegionBefore(op.getFunctionBody(), newFunc.getBody(), newFunc.end());
        return std::make_unique<OpAndValues>(newFunc.getOperation(), newFunc->getResults());
    }

    std::unique_ptr<OpAndValues> ReturnOpLowering::createNewOpAndValues(qnet::ReturnOp op,
                                                                        qnet::ReturnOp::Adaptor adaptor,
                                                                        ConversionPatternRewriter &rewriter) const {
        auto newReturn = rewriter.create<qmem::ReturnOp>(op.getLoc(), adaptor.getOperands());
        return std::make_unique<OpAndValues>(newReturn.getOperation(), newReturn->getResults());
    }

    /* Implementation of the operations that entangle qubits */
    std::unique_ptr<OpAndValues> EprsOpLowering::createNewOpAndValues(qnet::EprsOp op, qnet::EprsOp::Adaptor adaptor,
                                                                      ConversionPatternRewriter &rewriter) const {
        const Location loc = op.getLoc();
        // We first create a new qalloc operation
        auto newAllocOp = rewriter.create<qmem::QAllocOp>(loc);

        // Then we introduce the "init" operation as the "eprs" op
        auto newEprsOp = rewriter.create<qmem::EprsOp>(loc, newAllocOp.getResult(), adaptor.getRemoteAttr());
        // We replace all the uses of the old qubit with the new value (what was passed as "q" operand to eprs)
        rewriter.replaceAllUsesWith(op.getQout(), newEprsOp.getQ());
        return std::make_unique<OpAndValues>(newEprsOp.getOperation(), newAllocOp.getResult());
    }

    std::unique_ptr<OpAndValues>
    EprsMeasureOpLowering::createNewOpAndValues(qnet::EprsMeasureOp op, qnet::EprsMeasureOp::Adaptor adaptor,
                                                ConversionPatternRewriter &rewriter) const {
        const Location loc = op.getLoc();
        // We first create a new qalloc operation
        auto newAllocOp = rewriter.create<qmem::QAllocOp>(loc);
        Type mappedOutType = typeConverter->convertType(op.getOutcome().getType());
        auto newEprs =
                rewriter.create<qmem::EprsMeasureOp>(loc, mappedOutType, newAllocOp.getQ(), adaptor.getRemoteAttr());
        return std::make_unique<OpAndValues>(newEprs.getOperation(), newEprs->getResults());
    }

    /* Implementation of the lowering for the creation of new qubits */
    std::unique_ptr<OpAndValues> NewQubitLowering::createNewOpAndValues(qnet::NewQubitOp op,
                                                                        qnet::NewQubitOp::Adaptor adaptor,
                                                                        ConversionPatternRewriter &rewriter) const {
        const Location loc = op.getLoc();
        // First, we convert the qnet.qubit type into its mapped type
        Type mappedQubitType = typeConverter->convertType(op.getQout().getType());

        // Second, we create a QAllocOp instance that allocates a single qubit
        auto newAllocOp = rewriter.create<qmem::QAllocOp>(loc, mappedQubitType);

        // Third, we introduce the "init" operation required by the allocation
        auto newInitOp = rewriter.create<qmem::InitOp>(loc, newAllocOp.getResult());
        auto qalloc = dyn_cast<qmem::QAllocOp>(newInitOp.getQ().getDefiningOp());
        return std::make_unique<OpAndValues>(qalloc.getOperation(), qalloc->getResults());
    }

    std::unique_ptr<OpAndValues> RotateXLowering::createNewOpAndValues(qnet::RotXOp op, qnet::RotXOp::Adaptor adaptor,
                                                                       ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the output of the operation with
        // the mapped value of the "qin" operand of this operation
        Value adaptedQin = adaptor.getQin();
        Value adaptedAngle = adaptor.getAngle();
        rewriter.replaceAllUsesWith(op.getQout(), adaptedQin);
        auto newRotate = rewriter.create<qmem::RotateXOp>(op.getLoc(), adaptedQin, adaptedAngle);
        // This op mutates the qubit; in MIR we forward the input qubit value as the new SSA value.
        return std::make_unique<OpAndValues>(newRotate.getOperation(), newRotate.getQ());
    }

    std::unique_ptr<OpAndValues> RotateYLowering::createNewOpAndValues(qnet::RotYOp op, qnet::RotYOp::Adaptor adaptor,
                                                                       ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the output of the operation with
        // the mapped value of the "qin" operand of this operation
        Value adaptedQin = adaptor.getQin();
        Value adaptedAngle = adaptor.getAngle();
        rewriter.replaceAllUsesWith(op.getQout(), adaptedQin);
        auto newRotate = rewriter.create<qmem::RotateYOp>(op.getLoc(), adaptedQin, adaptedAngle);
        // This op mutates the qubit; in MIR we forward the input qubit value as the new SSA value.
        return std::make_unique<OpAndValues>(newRotate.getOperation(), newRotate.getQ());
    }

    std::unique_ptr<OpAndValues> RotateZLowering::createNewOpAndValues(qnet::RotZOp op, qnet::RotZOp::Adaptor adaptor,
                                                                       ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the output of the operation with
        // the mapped value of the "qin" operand of this operation
        Value adaptedQin = adaptor.getQin();
        Value adaptedAngle = adaptor.getAngle();
        rewriter.replaceAllUsesWith(op.getQout(), adaptedQin);
        auto newRotate = rewriter.create<qmem::RotateZOp>(op.getLoc(), adaptedQin, adaptedAngle);
        // This op mutates the qubit; in MIR we forward the input qubit value as the new SSA value.
        return std::make_unique<OpAndValues>(newRotate.getOperation(), newRotate.getQ());
    }

    std::unique_ptr<OpAndValues> HadamardLowering::createNewOpAndValues(qnet::HadamardOp op,
                                                                        qnet::HadamardOp::Adaptor adaptor,
                                                                        ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the output of the operation with
        // the mapped value of the "qin" operand of this operation
        Value adaptedQin = adaptor.getQin();
        rewriter.replaceAllUsesWith(op.getQout(), adaptedQin);
        auto newHadamard = rewriter.create<qmem::HadamardOp>(op.getLoc(), adaptedQin);
        // This op mutates the qubit; in MIR we forward the input qubit value as the new SSA value.
        return std::make_unique<OpAndValues>(newHadamard.getOperation(), newHadamard.getQ());
    }

    std::unique_ptr<OpAndValues> XLowering::createNewOpAndValues(qnet::XOp op, qnet::XOp::Adaptor adaptor,
                                                                 ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the output of the operation with
        // the mapped value of the "qin" operand of this operation
        Value adaptedQin = adaptor.getQin();
        rewriter.replaceAllUsesWith(op.getQout(), adaptedQin);
        auto newX = rewriter.create<qmem::XOp>(op.getLoc(), adaptedQin);
        // This op mutates the qubit; in MIR we forward the input qubit value as the new SSA value.
        return std::make_unique<OpAndValues>(newX.getOperation(), newX.getQ());
    }

    std::unique_ptr<OpAndValues> YLowering::createNewOpAndValues(qnet::YOp op, qnet::YOp::Adaptor adaptor,
                                                                 ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the output of the operation with
        // the mapped value of the "qin" operand of this operation
        Value adaptedQin = adaptor.getQin();
        rewriter.replaceAllUsesWith(op.getQout(), adaptedQin);
        auto newY = rewriter.create<qmem::YOp>(op.getLoc(), adaptedQin);
        // This op mutates the qubit; in MIR we forward the input qubit value as the new SSA value.
        return std::make_unique<OpAndValues>(newY.getOperation(), newY.getQ());
    }

    std::unique_ptr<OpAndValues> ZLowering::createNewOpAndValues(qnet::ZOp op, qnet::ZOp::Adaptor adaptor,
                                                                 ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the output of the operation with
        // the mapped value of the "qin" operand of this operation
        Value adaptedQin = adaptor.getQin();
        rewriter.replaceAllUsesWith(op.getQout(), adaptedQin);
        auto newZ = rewriter.create<qmem::ZOp>(op.getLoc(), adaptedQin);
        // This op mutates the qubit; in MIR we forward the input qubit value as the new SSA value.
        return std::make_unique<OpAndValues>(newZ.getOperation(), newZ.getQ());
    }

    std::unique_ptr<OpAndValues> MeasureLowering::createNewOpAndValues(qnet::MeasureOp op,
                                                                       qnet::MeasureOp::Adaptor adaptor,
                                                                       ConversionPatternRewriter &rewriter) const {
        // Measure yields an i1 type in both dialect spaces... this value does not need lowering, so we don't
        // need to remap the uses of the measure value.
        Value adaptedQin = adaptor.getQin();
        auto newMeasure = rewriter.create<qmem::MeasureOp>(op.getLoc(), rewriter.getI1Type(), adaptedQin);
        // qnet.measure and qmem.measure both produce an i1 outcome.
        // IMPORTANT: The replacement must be the *outcome* result. Returning the qubit
        // operand (or any other value) will create type/value mismatches and can cause
        // the conversion framework to insert UnrealizedConversionCastOps (e.g. before
        // arith.extui).
        return std::make_unique<OpAndValues>(newMeasure.getOperation(), newMeasure.getOutcome());
    }

    std::unique_ptr<OpAndValues> CNotLowering::createNewOpAndValues(qnet::CnotOp op, qnet::CnotOp::Adaptor adaptor,
                                                                    ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the outputs of the operation with
        // the mapped value of the respective "qin" operand of this operation
        // NOTE - For some reason (probably a misuse of MLIR or a bug) , if we use the rewriter object for
        // this purpose, it ends up on a SIGSEGV in the internals of the replacement of the operation
        // By using the "replaceAllUsesWith" directly from the value to replace, makes it work correctly
        Value adaptedQin0 = adaptor.getQin0();
        Value adaptedQin1 = adaptor.getQin1();
        op.getQout0().replaceAllUsesWith(adaptedQin0);
        op.getQout1().replaceAllUsesWith(adaptedQin1);
        auto newCnot = rewriter.create<qmem::CnotOp>(op.getLoc(), adaptedQin0, adaptedQin1);
        // This op mutates the qubit; in MIR we forward the input qubit value as the new SSA value.
        return std::make_unique<OpAndValues>(newCnot.getOperation(), newCnot.getOperands());
    }

    std::unique_ptr<OpAndValues> CzLowering::createNewOpAndValues(qnet::CzOp op, qnet::CzOp::Adaptor adaptor,
                                                                  ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the outputs of the operation with
        // the mapped value of the respective "qin" operand of this operation
        // NOTE - For some reason, if we use the rewriter object for this purpose, it ends up on a SIGSEGV
        // in the internals of the replacement of the operation
        Value adaptedQin0 = adaptor.getQin0();
        Value adaptedQin1 = adaptor.getQin1();
        op.getQout0().replaceAllUsesWith(adaptedQin0);
        op.getQout1().replaceAllUsesWith(adaptedQin1);
        auto newCz = rewriter.create<qmem::CzOp>(op.getLoc(), adaptedQin0, adaptedQin1);
        // This op mutates the qubit; in MIR we forward the input qubit value as the new SSA value.
        return std::make_unique<OpAndValues>(newCz.getOperation(), newCz.getOperands());
    }

    std::unique_ptr<OpAndValues> CRotXLowering::createNewOpAndValues(qnet::CrotXOp op, qnet::CrotXOp::Adaptor adaptor,
                                                                     ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the outputs of the operation with
        // the mapped value of the respective "qin" operand of this operation
        // NOTE - For some reason, if we use the rewriter object for this purpose, it ends up on a SIGSEGV
        // in the internals of the replacement of the operation
        Value adaptedQin0 = adaptor.getQin0();
        Value adaptedQin1 = adaptor.getQin1();
        Value adaptedAngle = adaptor.getAngle();
        op.getQout0().replaceAllUsesWith(adaptedQin0);
        op.getQout1().replaceAllUsesWith(adaptedQin1);
        auto newCRotX = rewriter.create<qmem::CrotXOp>(op.getLoc(), adaptedQin0, adaptedQin1, adaptedAngle);
        // This op mutates the qubit; in MIR we forward the input qubit value as the new SSA value.
        const auto opOperands = newCRotX->getOpOperands();
        OperandRange firstTwoOperands(opOperands.data(), 2);
        return std::make_unique<OpAndValues>(newCRotX.getOperation(), firstTwoOperands);
    }

    /* Implementation of the specific conversion between similar ops
     * These implementations do not need any special treatment, and mapping is quite straight forward */
    std::unique_ptr<OpAndValues> RemoteOpLowering::createNewOpAndValues(qnet::RemoteOp op,
                                                                        qnet::RemoteOp::Adaptor adaptor,
                                                                        ConversionPatternRewriter &rewriter) const {
        auto newRemote =
                rewriter.create<qmem::RemoteOp>(op.getLoc(), adaptor.getSymName(), adaptor.getSymVisibilityAttr());
        return std::make_unique<OpAndValues>(newRemote.getOperation(), newRemote->getResults());
    }

    std::unique_ptr<OpAndValues> SendIntsOpLowering::createNewOpAndValues(qnet::SendIntsOp op,
                                                                          qnet::SendIntsOp::Adaptor adaptor,
                                                                          ConversionPatternRewriter &rewriter) const {
        auto newSend = rewriter.create<qmem::SendIntsOp>(op.getLoc(), adaptor.getCin(), adaptor.getRemoteAttr());
        return std::make_unique<OpAndValues>(newSend.getOperation(), newSend->getResults());
    }

    std::unique_ptr<OpAndValues> RecvIntsOpLowering::createNewOpAndValues(qnet::RecvIntsOp op,
                                                                          qnet::RecvIntsOp::Adaptor adaptor,
                                                                          ConversionPatternRewriter &rewriter) const {
        // The output type is unchanged by this conversion;we will pass it "as is" to the new operation
        Type outType = typeConverter->convertType(op.getCout().getType());
        auto newRec = rewriter.create<qmem::RecvIntsOp>(op.getLoc(), outType, adaptor.getRemoteAttr(),
                                                        adaptor.getLengthAttr());
        return std::make_unique<OpAndValues>(newRec.getOperation(), newRec->getResults());
    }

    std::unique_ptr<OpAndValues> SendFloatsOpLowering::createNewOpAndValues(qnet::SendFloatsOp op,
                                                                            qnet::SendFloatsOp::Adaptor adaptor,
                                                                            ConversionPatternRewriter &rewriter) const {
        auto newSend = rewriter.create<qmem::SendFloatsOp>(op.getLoc(), adaptor.getCin(), adaptor.getRemoteAttr());
        return std::make_unique<OpAndValues>(newSend.getOperation(), newSend->getResults());
    }

    std::unique_ptr<OpAndValues> RecvFloatsOpLowering::createNewOpAndValues(qnet::RecvFloatsOp op,
                                                                            qnet::RecvFloatsOp::Adaptor adaptor,
                                                                            ConversionPatternRewriter &rewriter) const {
        // The output type is unchanged by this conversion;we will pass it "as is" to the new operation
        Type outType = typeConverter->convertType(op.getCout().getType());
        auto newSend = rewriter.create<qmem::RecvFloatsOp>(op.getLoc(), outType, adaptor.getRemoteAttr(),
                                                           adaptor.getLengthAttr());
        return std::make_unique<OpAndValues>(newSend.getOperation(), newSend->getResults());
    }

    std::unique_ptr<OpAndValues> SendIntOpLowering::createNewOpAndValues(qnet::SendIntOp op,
                                                                         qnet::SendIntOp::Adaptor adaptor,
                                                                         ConversionPatternRewriter &rewriter) const {
        auto newSend = rewriter.create<qmem::SendIntOp>(op.getLoc(), adaptor.getCin(), adaptor.getRemoteAttr());
        return std::make_unique<OpAndValues>(newSend.getOperation(), newSend->getResults());
    }

    std::unique_ptr<OpAndValues> RecvIntOpLowering::createNewOpAndValues(qnet::RecvIntOp op,
                                                                         qnet::RecvIntOp::Adaptor adaptor,
                                                                         ConversionPatternRewriter &rewriter) const {
        // The output type is unchanged by this conversion;we will pass it "as is" to the new operation
        Type outType = typeConverter->convertType(op.getCout().getType());
        auto newRec = rewriter.create<qmem::RecvIntOp>(op.getLoc(), outType, adaptor.getRemoteAttr());
        return std::make_unique<OpAndValues>(newRec.getOperation(), newRec->getResults());
    }

    std::unique_ptr<OpAndValues> SendFloatOpLowering::createNewOpAndValues(qnet::SendFloatOp op,
                                                                           qnet::SendFloatOp::Adaptor adaptor,
                                                                           ConversionPatternRewriter &rewriter) const {
        auto newSend = rewriter.create<qmem::SendFloatOp>(op.getLoc(), adaptor.getCin(), adaptor.getRemoteAttr());
        return std::make_unique<OpAndValues>(newSend.getOperation(), newSend->getResults());
    }

    std::unique_ptr<OpAndValues> RecvFloatOpLowering::createNewOpAndValues(qnet::RecvFloatOp op,
                                                                           qnet::RecvFloatOp::Adaptor adaptor,
                                                                           ConversionPatternRewriter &rewriter) const {
        // The output type is unchanged by this conversion;we will pass it "as is" to the new operation
        Type outType = typeConverter->convertType(op.getCout().getType());
        auto newSend = rewriter.create<qmem::RecvFloatOp>(op.getLoc(), outType, adaptor.getRemoteAttr());
        return std::make_unique<OpAndValues>(newSend.getOperation(), newSend->getResults());
    }

    LogicalResult ScfIfLowering::matchAndRewrite(scf::IfOp op, PatternRewriter &rewriter) const {
        // This is the "lowering" of scf.if.
        // The idea here is that we need to get rid of the !qnet.qubit returned type (if any)
        // and adjust the scf.yield operations that return !qnet.qubit values

        LLVM_DEBUG(llvm::dbgs() << "Analyzing:\n" << op << "\n*************\n");
        LLVM_DEBUG(llvm::dbgs() << "Start:\n" << op->getParentOfType<ModuleOp>() << "\n*************\n");

        // Analyze the original yielded values type, and build a structure containing the
        // index of the yielded value, and the qubit value that it represents
        auto origThenYieldOp = op.thenYield();
        auto origElseYieldOp = op.elseYield();
        DenseMap<uint32_t, Value> qubitYieldIndexes;

        if (failed(helpers::analyzeYieldOps(origThenYieldOp, origElseYieldOp, qubitYieldIndexes))) {
            op->emitError(R"(SCF If lowering: "then" ands "else" branches could not be analyzed)");
            return failure();
        }

        SmallVector<Type> newYieldedTypes;
        DenseMap<uint32_t, uint32_t> valuesIndexesMap;
        for (uint32_t resIdx = 0, newResIdx = 0; resIdx < op.getResults().size(); resIdx++) {
            // qubitYieldIndexes contains the indexes of values that represent a qubit so,
            // if the index is there, that type should *not* be a result type of the new scf.if
            if (qubitYieldIndexes.contains(resIdx)) {
                continue;
            }
            newYieldedTypes.push_back(op.getResult(resIdx).getType());
            // We still map the old index value with the new one
            valuesIndexesMap.try_emplace(resIdx, newResIdx++);
        }

        // Create a new yield operation that does not yield qubit types
        auto newIfOp = rewriter.create<scf::IfOp>(op.getLoc(), newYieldedTypes, op.getCondition(),
                                                  /*addThenBlock=*/false, /*addElseBlock=*/false);

        LLVM_DEBUG(llvm::dbgs() << "New If:\n" << newIfOp->getParentOfType<ModuleOp>() << "\n*************\n");

        // Move the then/else regions, adjusting types as needed.
        rewriter.inlineRegionBefore(op.getThenRegion(), newIfOp.getThenRegion(), newIfOp.getThenRegion().end());
        rewriter.inlineRegionBefore(op.getElseRegion(), newIfOp.getElseRegion(), newIfOp.getElseRegion().end());

        auto newThenYieldOp = newIfOp.thenYield();
        auto newElseYieldOp = newIfOp.elseYield();

        if (failed(helpers::replaceYieldOps(newThenYieldOp, newElseYieldOp, qubitYieldIndexes, rewriter))) {
            op->emitError(R"(SCF If lowering: "then" ands "else" branches could not be analyzed)");
            return failure();
        }

        LLVM_DEBUG(llvm::dbgs() << "Blocks corrected:\n"
                                << newIfOp->getParentOfType<ModuleOp>() << "\n*************\n");

        // Delete any users of the deleted result value - We might want to
        for (auto &[yieldIdx, yieldVal] : qubitYieldIndexes) {
            Value originalValueToReplace = op.getResult(yieldIdx);
            LLVM_DEBUG(llvm::dbgs() << "Replacing users of: " << originalValueToReplace << "\n");
            for (Operation *valUse : originalValueToReplace.getUsers()) {
                if (failed(helpers::replaceUnrealizedCastOps(valUse, yieldVal, rewriter))) {
                    return failure();
                }
            }
        }

        for (auto [oldIdx, newIdx] : valuesIndexesMap) {
            rewriter.replaceAllUsesWith(op.getResult(oldIdx), newIfOp.getResult(newIdx));
        }
        LLVM_DEBUG(llvm::dbgs() << "After:\n" << newIfOp->getParentOfType<ModuleOp>() << "\n*************\n");
        rewriter.eraseOp(op.getOperation());

        helpers::fixEmptySCFBranchIfNeeded(newIfOp, rewriter);

        return LogicalResult::success();
    }
} // namespace qoala::conversion::hir
