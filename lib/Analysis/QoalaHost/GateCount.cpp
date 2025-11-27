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

        llvm::DenseMap<Value, std::string> qubitId;

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
            
            mlir::DenseMap<mlir::Value, mlir::Value> calleeToCaller;

            // Map call operands to callee block arguments if anygetBlockArgForCallerValue
            for (auto [callArg, blockArg] : llvm::zip(operands, blockArgs)) {
                calleeToCaller[blockArg] = callArg;
            }

            auto returnOp = *entryBlock.getOps<netqasm::ReturnOp>().begin();

            // Map from retunred Values inside calle to returned values in main func
            for (auto returnVal : llvm::enumerate(returnOp->getOperands())) {
                calleeToCaller[returnVal.value()] = callOp.getResult(returnVal.index());
            }

            for (auto &op : entryBlock) {
                if (llvm::isa<netqasm::QInitOp, netqasm::EprsOp>(op)) {
                    auto newQubit = op.getOperands().front();
                    if (calleeToCaller.contains(newQubit)) {
                        std::string qId = blckId + "::" + std::to_string(opIdx);
                        qubitId[calleeToCaller[newQubit]] = qId;
                        detailedOneQubitGateCount[qId] = 0;
                        detailedTwoQubitGateCount[qId] = 0;
                        detailedGateCount[qId] = 0;
                    }
                } else if (llvm::isa<netqasm::ifaces::SingleQubitOp>(op)) {
                    gateCount++;
                    oneQubitGateCount++;
                    auto qId = qubitId.at(calleeToCaller.at(op.getOperands().front()));
                    detailedGateCount[qId] += 1;
                    detailedOneQubitGateCount[qId] += 1;
                } else if (llvm::isa<netqasm::ifaces::DualQubitOp>(op)) {
                    gateCount++;
                    twoQubitGateCount++;
                    for (auto operand : op.getOperands()) {
                        if (calleeToCaller.contains(operand)){
                            auto qId = qubitId.at(calleeToCaller.at(operand));
                            detailedGateCount[qId] += 1;
                            detailedTwoQubitGateCount[qId] += 1;
                        }
                    }
                }
                ++opIdx;
            }
        }

    }

} // namespace qoala::analysis::gatecount
