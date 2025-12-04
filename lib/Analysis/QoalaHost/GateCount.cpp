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
using namespace qoala::dialects;

namespace qoala::analysis::gatecount {

    /// Map call operands/results to callee block args/returns.
    static mlir::DenseMap<mlir::Value, mlir::Value> mapCallArgsToCallee(qoalahost::CallOp callOp,
                                                                        FunctionOpInterface callee) {
        mlir::DenseMap<mlir::Value, mlir::Value> mapping;

        // Map input args
        auto operands = callOp.getOperands();
        auto entryArgs = callee.front().getArguments();
        for (auto [callArg, blockArg] : llvm::zip(operands, entryArgs)) {
            mapping.try_emplace(blockArg, callArg);
        }

        // Map return values
        auto returnOps = callee.front().getOps<netqasm::ReturnOp>();
        assert(!returnOps.empty() && "callee must have a ReturnOp");
        netqasm::ReturnOp returnOp = *returnOps.begin();
        for (auto retVal : llvm::enumerate(returnOp->getOperands())) {
            mapping.try_emplace(retVal.value(), callOp.getResult(retVal.index()));
        }

        return mapping;
    }

    /// Flush accumulated 1-qubit gates for a qubit into global counters.
    static void flushTempOneQubitCount(const std::string &qId,
                                       std::unordered_map<std::string, uint32_t> &tempOneQubitGateCount,
                                       uint32_t &gateCount, uint32_t &oneQubitGateCount,
                                       std::unordered_map<std::string, uint32_t> &detailedGateCount,
                                       std::unordered_map<std::string, uint32_t> &detailedOneQubitGateCount) {
        auto it = tempOneQubitGateCount.find(qId);
        if (it == tempOneQubitGateCount.end() || it->second == 0) {
            return;
        }

        uint32_t tempCount = it->second;
        gateCount += tempCount;
        oneQubitGateCount += tempCount;
        detailedGateCount[qId] += tempCount;
        detailedOneQubitGateCount[qId] += tempCount;
        it->second = 0;
    }

    QoalaHostGateCount::QoalaHostGateCount(Operation *op) {
        // Count the gates qubit-wise, and grouped by one-qubit and two-qubit gates
        // as they have different error rates and so contribute differently
        // to the fidelity of each qubit

        LLVM_DEBUG(llvm::dbgs() << "Running QoalaHostGateCountPass\n");

        // Map Values to qubit ids, ids should be the same as in qubit lifetimes
        llvm::DenseMap<Value, std::string> qubitIds;
        // Gate counts at this level are used for the ESP pass,
        // consequently gates should be counted only when they can affect the final ESP.
        // Temp counts accumulate 1-qubit gates until measurement/two-qubit interaction to model active qubit lifetime.
        std::unordered_map<std::string, uint32_t> tempOneQubitGateCount;

        auto mainFuncs = dyn_cast<ModuleOp>(op).getOps<qoalahost::MainFuncOp>();

        assert(!mainFuncs.empty() && "No main function found in module.");

        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();

        // Walk through all operations in the main function
        for (auto callOp : mainFunc.getOps<qoalahost::CallOp>()) {
            // The call itself counts as an op, start from 1
            uint32_t opIdx = 1;

            Block *block = callOp->getBlock();
            auto blkMeta = dyn_cast<qoalahost::BlkMeta>(block->front());
            std::string blckId = blkMeta.getBlockId().str();

            // Get the callee
            auto callee = callOp.getCalleeOperation<FunctionOpInterface>();

            auto calleeToCaller = mapCallArgsToCallee(callOp, callee);

            // Count gates
            for (auto &op : callee.front()) {
                llvm::TypeSwitch<mlir::Operation *>(&op)
                        .Case<netqasm::MeasureOp>([&](auto meas) {
                            // Do not count measure ops
                            // In the future, could look into readout error and count them separetly

                            // Update counts with temp one-qubit gate count
                            auto qId = qubitIds.at(calleeToCaller.at(meas.getOperation()->getOperands().front()));

                            flushTempOneQubitCount(qId, tempOneQubitGateCount, gateCount, oneQubitGateCount,
                                                   detailedGateCount, detailedOneQubitGateCount);
                        })
                        .Case<netqasm::QInitOp, netqasm::EprsOp>([&](auto init) {
                            // Do not count initialization as a gate,
                            // used as a reference point to start raking a qubit and its gates
                            auto newQubit = init.getOperation()->getOperands().front();
                            if (calleeToCaller.contains(newQubit)) {
                                std::string qId = blckId + "::" + std::to_string(opIdx);
                                qubitIds[calleeToCaller[newQubit]] = qId;
                                tempOneQubitGateCount[qId] = 0;
                                detailedOneQubitGateCount[qId] = 0;
                                detailedTwoQubitGateCount[qId] = 0;
                                detailedGateCount[qId] = 0;
                            }
                        })
                        .Case<netqasm::ifaces::SingleQubitOp>([&](auto oneQubitOp) {
                            // Update temp one-qubit gate count
                            auto qId = qubitIds.at(calleeToCaller.at(oneQubitOp.getOperation()->getOperands().front()));
                            tempOneQubitGateCount[qId] += 1;
                        })
                        .Case<netqasm::ifaces::DualQubitOp>([&](auto twoQubitOp) {
                            gateCount++;
                            twoQubitGateCount++;
                            for (auto operand : twoQubitOp.getOperation()->getOperands()) {
                                if (calleeToCaller.contains(operand)) {
                                    auto qId = qubitIds.at(calleeToCaller.at(operand));
                                    detailedGateCount[qId] += 1;
                                    detailedTwoQubitGateCount[qId] += 1;

                                    flushTempOneQubitCount(qId, tempOneQubitGateCount, gateCount, oneQubitGateCount,
                                                           detailedGateCount, detailedOneQubitGateCount);
                                }
                            }
                        });
                // Increase op index for the current call block
                ++opIdx;
            }
        }
    }

} // namespace qoala::analysis::gatecount
