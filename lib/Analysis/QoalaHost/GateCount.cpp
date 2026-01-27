#include "Analysis/QoalaHost/GateCount.h"
#include "Analysis/NetQASM/Helpers.h"
#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/QoalaHost/Passes.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Debug.h"
#include "mlir/IR/AsmState.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"

#define DEBUG_TYPE "qoalahost-gate-count"

using namespace mlir;
using namespace qoala;

namespace qoala::analysis::gatecount {

    /// Map call operands/results to callee block args/returns.
    static mlir::DenseMap<mlir::Value, mlir::Value> mapReturnValToCaller(dialects::qoalahost::CallOp callOp,
                                                                         FunctionOpInterface callee) {
        mlir::DenseMap<mlir::Value, mlir::Value> returnedValuesMap;

        // Map return values
        auto returnOps = callee.front().getOps<dialects::netqasm::ReturnOp>();
        assert(!returnOps.empty() && "callee must have a ReturnOp");
        dialects::netqasm::ReturnOp returnOp = *returnOps.begin();
        for (auto retVal : llvm::enumerate(returnOp->getOperands())) {
            returnedValuesMap.try_emplace(retVal.value(), callOp.getResult(retVal.index()));
        }

        return returnedValuesMap;
    }

    /// Flush accumulated 1-qubit gates for a qubit into global counters.
    static void flushTempOneQubitCount(const llvm::StringRef &qId, llvm::StringMap<uint32_t> &tempOneQubitGateCount,
                                       uint32_t &gateCount, uint32_t &oneQubitGateCount,
                                       llvm::StringMap<uint32_t> &detailedGateCount,
                                       llvm::StringMap<uint32_t> &detailedOneQubitGateCount) {

        LLVM_DEBUG(llvm::dbgs() << "Looking for qubit " << qId << " in temp one-qubit gate counts.\n");
        if (tempOneQubitGateCount.contains(qId)) {
            LLVM_DEBUG(llvm::dbgs() << "Found qubit " << qId << " in temp one-qubit gate counts.\n");
            uint32_t tempCount = tempOneQubitGateCount.at(qId);
            if (tempCount != 0) {
                LLVM_DEBUG(llvm::dbgs() << "Flushing one-qubit gates count on qubit " << qId << ".\n");
                gateCount += tempCount;
                oneQubitGateCount += tempCount;
                detailedGateCount[qId] += tempCount;
                detailedOneQubitGateCount[qId] += tempCount;
                tempOneQubitGateCount[qId] = 0;
            }
        }

        return;
    }

    static std::optional<mlir::Value>
    getCallValFromCallee(const mlir::Value qubit, const analysis::netqasm::ArgValueMap &valuesArgMap,
                         const mlir::DenseMap<mlir::Value, mlir::Value> &returnedValuesMap) {
        if (auto blockArg = dyn_cast<BlockArgument>(qubit)) {
            return valuesArgMap.getCallerValueForArg(blockArg);
        }
        if (returnedValuesMap.contains(qubit)) {
            return returnedValuesMap.at(qubit);
        }

        return std::nullopt;
    }

    static llvm::StringRef getQubitID(const mlir::Value &operand, const llvm::DenseMap<Value, std::string> &qubitIDs,
                                      const analysis::netqasm::ArgValueMap &valuesArgMap,
                                      const mlir::DenseMap<mlir::Value, mlir::Value> &returnedValuesMap) {
        auto callVal = getCallValFromCallee(operand, valuesArgMap, returnedValuesMap);
        assert(callVal.has_value() && "Untracked qubit!");
        return qubitIDs.at(callVal.value());
    }

    QoalaHostGateCount::QoalaHostGateCount(Operation *op) {
        /**
         * Count the gates qubit-wise, and grouped by one-qubit and two-qubit gates
         * as they have different error rates and so contribute differently
         * to the fidelity of each qubit
         */

        // Current implementation is tightly coupled with fidelity estimation, i.e.,
        // gate count is tracked up to measurement or last two-qubit op.
        // In future could change to track every qubit up to last quantum op.

        LLVM_DEBUG(llvm::dbgs() << "Running QoalaHostGateCountPass\n");

        // Map Values to qubit ids, ids should be the same as in QubitLifetime, order of operations is important
        llvm::DenseMap<Value, std::string> qubitIds;
        // Gate counts at this level are used for the ESP pass,
        // consequently gates should be counted only when they can affect the final ESP.
        // Temp counts accumulate 1-qubit gates until measurement/two-qubit interaction to model active qubit lifetime.
        llvm::StringMap<uint32_t> tempOneQubitGateCount;

        auto mainFuncs = dyn_cast<ModuleOp>(op).getOps<dialects::qoalahost::MainFuncOp>();

        assert(!mainFuncs.empty() && "No main function found in module.");

        dialects::qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();

        // Walk through all operations in the main function
        mainFunc.walk([&](dialects::qoalahost::CallOp callOp) {
            // The call itself counts as an op, start from 1
            uint32_t opIdx = 1;

            Block *block = callOp->getBlock();
            auto blkMeta = dyn_cast<dialects::qoalahost::BlkMeta>(block->front());
            std::string blckId = blkMeta.getBlockId().str();

            // Get the callee
            auto callee = callOp.getCalleeOperation<FunctionOpInterface>();

            analysis::netqasm::ArgValueMap valuesArgMap =
                    analysis::netqasm::getRoutineArgValues(callee, callOp.getOperands());

            auto returnedValuesMap = mapReturnValToCaller(callOp, callee);

            // Count gates
            callee.walk([&](mlir::Operation *op) {
                llvm::TypeSwitch<mlir::Operation *>(op)
                        .Case<dialects::netqasm::MeasureOp>([&](auto meas) {
                            // Do not count measure ops
                            // In the future, could look into readout error and count them separetly

                            // Update counts with temp one-qubit gate count
                            auto operand = meas.getOperation()->getOperands().front();
                            auto qId = getQubitID(operand, qubitIds, valuesArgMap, returnedValuesMap);

                            LLVM_DEBUG(llvm::dbgs() << "Found Meas op on qubit " << qId << ".\n");

                            flushTempOneQubitCount(qId, tempOneQubitGateCount, gateCount, oneQubitGateCount,
                                                   detailedGateCount, detailedOneQubitGateCount);
                        })
                        .Case<dialects::netqasm::QInitOp, dialects::netqasm::EprsOp>([&](auto init) {
                            // Do not count initialization as a gate,
                            // used as a reference point to start traking a qubit and its gates
                            auto operand = init.getOperation()->getOperands().front();
                            std::string qId = blckId + "::" + std::to_string(opIdx);
                            LLVM_DEBUG(llvm::dbgs() << "Found initialization of qubit " << qId << ".\n");
                            if (auto newQubit = getCallValFromCallee(operand, valuesArgMap, returnedValuesMap)) {
                                qubitIds[newQubit.value()] = qId;
                            } else {
                                LLVM_DEBUG(llvm::dbgs() << "Qubit " << qId << " is initialized but not returned.\n");
                                returnedValuesMap[newQubit.value()] = newQubit.value();
                                qubitIds[newQubit.value()] = qId;
                            }
                            tempOneQubitGateCount[qId] = 0;
                            detailedOneQubitGateCount[qId] = 0;
                            detailedTwoQubitGateCount[qId] = 0;
                            detailedGateCount[qId] = 0;
                        })
                        .Case<dialects::netqasm::ifaces::SingleQubitOp>([&](auto oneQubitOp) {
                            // Update temp one-qubit gate count
                            auto operand = oneQubitOp.getOperation()->getOperands().front();
                            auto qId = getQubitID(operand, qubitIds, valuesArgMap, returnedValuesMap);

                            LLVM_DEBUG(llvm::dbgs() << "Found one-qubit gate " << oneQubitOp.getOperation() << ".\n");
                            LLVM_DEBUG(llvm::dbgs() << "Updating one-qubit gate count on qubit " << qId << ".\n");

                            tempOneQubitGateCount[qId] += 1;
                        })
                        .Case<dialects::netqasm::ifaces::DualQubitOp>([&](auto twoQubitOp) {
                            gateCount++;
                            twoQubitGateCount++;
                            LLVM_DEBUG(llvm::dbgs() << "Found two-qubit gate " << twoQubitOp.getOperation() << ".\n");
                            for (auto operand : twoQubitOp.getOperation()->getOperands()) {
                                auto qId = getQubitID(operand, qubitIds, valuesArgMap, returnedValuesMap).str();
                                LLVM_DEBUG(llvm::dbgs() << "Updating two-qubit gate count on qubit " << qId << ".\n");
                                detailedGateCount[qId] += 1;
                                detailedTwoQubitGateCount[qId] += 1;

                                flushTempOneQubitCount(qId, tempOneQubitGateCount, gateCount, oneQubitGateCount,
                                                       detailedGateCount, detailedOneQubitGateCount);
                            }
                        });
                // Increase op index for the current call block
                ++opIdx;
            });
        });
    }

} // namespace qoala::analysis::gatecount
