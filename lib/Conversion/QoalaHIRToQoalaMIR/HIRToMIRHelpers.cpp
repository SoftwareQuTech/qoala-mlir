#include <mlir/Dialect/Vector/IR/VectorOps.h.inc>

#include "Conversion/QoalaHIRToQoalaMIR/QoalaHIRToQoalaMIRPatterns.h"
#include "llvm/ADT/TypeSwitch.h"

#define DEBUG_TYPE "hir-to-qoala-mir-helpers"

using namespace mlir;

namespace qoala::conversion::hir::helpers {
    LogicalResult replaceUnrealizedCastOps(Operation *user, const Value &replaceWith, PatternRewriter &rewriter) {
        LLVM_DEBUG(llvm::dbgs() << "replacing: " << *user << "\n  with: " << replaceWith << "\n");
        if (auto castOp = dyn_cast<UnrealizedConversionCastOp>(user)) {
            rewriter.replaceAllUsesWith(castOp.getResults(), replaceWith);
            rewriter.eraseOp(user);
            return success();
        }
        if (auto yieldOp = dyn_cast<scf::YieldOp>(user)) {
            rewriter.setInsertionPoint(yieldOp);
            rewriter.create<scf::YieldOp>(yieldOp.getLoc(), replaceWith);
            rewriter.eraseOp(user);
            return success();
        }
        user->emitError("Trying to replace the users of an op which is not an Unrealized Cast");
        return failure();
    }

    LogicalResult replaceYieldOps(scf::YieldOp &thenYield, scf::YieldOp &elseYield,
                                  const DenseMap<uint32_t, Value> &qubitYieldIndexes, PatternRewriter &rewriter) {
        SmallVector<Value> thenValuesToYield;
        for (uint32_t i = 0; i < thenYield.getNumOperands(); i++) {
            if (!qubitYieldIndexes.contains(i)) {
                thenValuesToYield.push_back(thenYield.getOperand(i));
            }
        }
        rewriter.setInsertionPoint(thenYield);
        rewriter.replaceOpWithNewOp<scf::YieldOp>(thenYield, thenValuesToYield);

        SmallVector<Value> elseValuesToYield;
        for (uint32_t i = 0; i < thenYield.getNumOperands(); i++) {
            if (!qubitYieldIndexes.contains(i)) {
                elseValuesToYield.push_back(elseYield.getOperand(i));
            }
        }
        rewriter.setInsertionPoint(elseYield);
        rewriter.replaceOpWithNewOp<scf::YieldOp>(elseYield, elseValuesToYield);
        return success();
    }

    static std::optional<Value> getOriginalQAllocValue(const Value &val) {
        Operation *definingOp = val.getDefiningOp();
        if (definingOp == nullptr) {
            return std::nullopt;
        }
        return llvm::TypeSwitch<Operation *, std::optional<Value>>(definingOp)
                .Case([](dialects::qmem::QAllocOp qAlloc) -> std::optional<Value> { return {qAlloc.getResult()}; })
                .Case([](scf::IfOp ifOp) -> std::optional<Value> {
                    scf::YieldOp thenYield = ifOp.thenYield();
                    scf::YieldOp elseYield = ifOp.elseYield();
                    std::optional<Value> thenVal = getOriginalQAllocValue(thenYield.getOperand(0));
                    if (thenVal.has_value()) {
                        return thenVal;
                    }
                    std::optional<Value> elseVal = getOriginalQAllocValue(elseYield.getOperand(0));
                    if (elseVal.has_value()) {
                        return elseVal;
                    }
                    return std::nullopt;
                })
                .Default([](Operation *op) -> std::optional<Value> {
                    for (Value operand : op->getOperands()) {
                        const std::optional<Value> operandOrigQAlloc = getOriginalQAllocValue(operand);
                        if (operandOrigQAlloc.has_value()) {
                            return operandOrigQAlloc;
                        }
                    }
                    return std::nullopt;
                });
    }

    LogicalResult analyzeYieldOps(scf::YieldOp &thenYield, scf::YieldOp &elseYield,
                                  DenseMap<uint32_t, Value> &matchedIdx) {
        // Known limitation. Let's consider this program;
        // %0 = qmem.qalloc() : i32
        // qmem.qinit(%0)
        // %1 = qmem.qalloc() : i32
        // qmem.qinit(%1)
        // %2 = arith.cmpi....
        // %3 = scf.if %2 {
        //   scf.yield %1 : i32
        // } else {
        //   scf.yield %0 : i32
        // }
        // %4 = qmem.measure %3 : i1
        // In this program, %3 becomes an alias of either %0 or %1.
        // This type of program is not supported by this algorithm, so we ensure that
        // both terminators of the branch yield the *same value*.
        LLVM_DEBUG(llvm::dbgs() << "Analyzing:\n  thenYield: " << thenYield << "\n  elseYield: " << elseYield << "\n");
        assert(thenYield.getNumOperands() == elseYield.getNumOperands() && "Number of yielded values do not match");
        for (uint32_t operandIdx = 0; operandIdx < thenYield.getNumOperands(); operandIdx++) {
            Value thenValue = thenYield.getOperand(operandIdx);
            Value elseValue = elseYield.getOperand(operandIdx);

            auto qAllocFromThenYield = getOriginalQAllocValue(thenValue);
            auto qAllocFromElseYield = getOriginalQAllocValue(elseValue);

            LLVM_DEBUG(llvm::dbgs() << "Discovered qAllocs:\n  * qAlloc then: " << qAllocFromThenYield
                                    << "\n  * qAlloc else: " << qAllocFromElseYield << "\n");

            // Both sides need to match to a qubit, and they need to match to the *same* qubit.
            if (!qAllocFromThenYield.has_value() || !qAllocFromElseYield.has_value() ||
                *qAllocFromThenYield != *qAllocFromElseYield) {
                continue;
            }
            // Both yielded values represent the same qubit; mark the index of the yield.
            matchedIdx.try_emplace(operandIdx, thenValue);
        }
        return success();
    }
} // namespace qoala::conversion::hir::helpers
