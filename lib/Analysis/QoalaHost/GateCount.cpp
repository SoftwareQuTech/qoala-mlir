#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/QoalaHost/Passes.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/Debug.h"
#include "mlir/IR/AsmState.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"

#define DEBUG_TYPE "qoalahost-gate-count"

using namespace mlir;
using namespace qoala::dialects;

namespace qoala::analysis::gatecount {

    QoalaHostGateCount::QoalaHostGateCount(Operation *op) {
        // Count the gates qubit-wise, and grouped by one-qubit and two-qubit gates
        // as they have different error rates and so contribute differently
        // to the fidelity of each qubit

        LLVM_DEBUG(llvm::dbgs() << "Running QoalaHostGateCountPass\n");

        // Map Values to qubit ids, ids should be the same as in qubit lifetimes
        llvm::DenseMap<Value, std::string> qubitId;
        // Temporary one-qubit gate counts, add them to final counts only if qubits are measured
        // or when a two-qubit gate is encountered. Gate counts at this level are used
        // for the ESP pass, consequently gates should be counted only when they can
        // affect the final ESP, meaning that the interested qubit si either measured or interacts with other qubits.
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

            // Get the operands (qubit arguments passed to the call) if any
            auto operands = callOp.getOperands();

            // Get the entry block and its arguments if any
            auto &entryBlock = callee.front();
            auto blockArgs = entryBlock.getArguments();

            // Map call operands to callee block arguments if anygetBlockArgForCallerValue
            mlir::DenseMap<mlir::Value, mlir::Value> calleeToCaller;

            for (auto [callArg, blockArg] : llvm::zip(operands, blockArgs)) {
                calleeToCaller[blockArg] = callArg;
            }

            // Map from retunred Values inside calle to returned values in main func
            auto returnOp = *entryBlock.getOps<netqasm::ReturnOp>().begin();

            for (auto returnVal : llvm::enumerate(returnOp->getOperands())) {
                calleeToCaller[returnVal.value()] = callOp.getResult(returnVal.index());
            }

            // Count gates
            for (auto &op : entryBlock) {
                // Do not count measure ops
                // In the future, could look into readout error and count them separetly
                if (llvm::isa<netqasm::MeasureOp>(op)) {
                    // Update counts with temp one-qubit gate count
                    auto qId = qubitId.at(calleeToCaller.at(op.getOperands().front()));
                    gateCount += tempOneQubitGateCount[qId];
                    oneQubitGateCount += tempOneQubitGateCount[qId];
                    detailedGateCount[qId] += tempOneQubitGateCount[qId];
                    detailedOneQubitGateCount[qId] += tempOneQubitGateCount[qId];
                }
                if (llvm::isa<netqasm::QInitOp, netqasm::EprsOp>(op)) {
                    // Do not count initialization as a gate,
                    // used as a reference point to start raking a qubit and its gates
                    auto newQubit = op.getOperands().front();
                    if (calleeToCaller.contains(newQubit)) {
                        std::string qId = blckId + "::" + std::to_string(opIdx);
                        qubitId[calleeToCaller[newQubit]] = qId;
                        tempOneQubitGateCount[qId] = 0;
                        detailedOneQubitGateCount[qId] = 0;
                        detailedTwoQubitGateCount[qId] = 0;
                        detailedGateCount[qId] = 0;
                    }
                } else if (llvm::isa<netqasm::ifaces::SingleQubitOp>(op)) {
                    // Update temp one-qubit gate count
                    auto qId = qubitId.at(calleeToCaller.at(op.getOperands().front()));
                    tempOneQubitGateCount[qId] += 1;
                } else if (llvm::isa<netqasm::ifaces::DualQubitOp>(op)) {
                    gateCount++;
                    twoQubitGateCount++;
                    for (auto operand : op.getOperands()) {
                        if (calleeToCaller.contains(operand)) {
                            auto qId = qubitId.at(calleeToCaller.at(operand));
                            detailedGateCount[qId] += 1;
                            detailedTwoQubitGateCount[qId] += 1;

                            // Update counts with temp one-qubit gate count
                            gateCount += tempOneQubitGateCount[qId];
                            oneQubitGateCount += tempOneQubitGateCount[qId];
                            detailedGateCount[qId] += tempOneQubitGateCount[qId];
                            detailedOneQubitGateCount[qId] += tempOneQubitGateCount[qId];
                            tempOneQubitGateCount[qId] = 0;
                        }
                    }
                }
                // Increase op index for the current call block
                ++opIdx;
            }
        }
    }

} // namespace qoala::analysis::gatecount
