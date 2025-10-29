#include "Analysis/QNet/Helpers.h"
#include "Dialect/QNet/Passes.h"
#include "Dialect/QNet/QNet.h"
#include "llvm/Support/Debug.h"
#include "mlir/IR/AsmState.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"

#define DEBUG_TYPE "qnet-gate-count"

using namespace mlir;
using namespace qoala::dialects::qnet;

namespace qoala::analysis::gatecount {

    bool isQuantumOp(mlir::Operation *op) {
        return llvm::isa<NewQubitOp, RotXOp, RotYOp, RotZOp, HadamardOp, CnotOp, CzOp, CrotXOp, MeasureOp, EprsOp, EprsMeasureOp>(op);
    }

    bool isOneQubitOp(mlir::Operation *op) {
        return llvm::isa<NewQubitOp, RotXOp, RotYOp, RotZOp, HadamardOp, MeasureOp, EprsOp, EprsMeasureOp>(op);
    }

    QNetGateCount::QNetGateCount(Operation *op) {
        llvm::DenseMap<Value, std::vector<Operation*>> qubitsToOps;
        llvm::DenseMap<Value, Value> qubitsInitToOpRes;

        if (ModuleOp module = llvm::dyn_cast<ModuleOp>(op)) {
            // Walk over all ops in the module.
            module.walk([&](Operation *op) {
                if (llvm::isa<NewQubitOp, EprsOp>(op)) {
                    ++gateCount;
                    llvm::outs() << "New Qubit Op for qubit: " << op->getName().getStringRef() << ".\n";
                    for (Value result : op->getResults()) {
                        qubitsInitToOpRes[result] = result;
                        qubitsToOps.try_emplace(result);
                        qubitsToOps[result].push_back(op);
                    }
                } else {
                    if (isQuantumOp(op)) {
                        ++gateCount;
                        // For each operand, find its lineage ID and propagate it to the
                        // corresponding result.
                        for (auto it : llvm::enumerate(op->getOperands())) {
                            Value operand = it.value();
                            unsigned index = it.index();

                            // Check if the operand is a qubit we are tracking
                            auto foundIt = qubitsInitToOpRes.find(operand);
                            if (foundIt != qubitsInitToOpRes.end()) {
                                Value lineageId = foundIt->second;
                                
                                // Add this operation to the history of the qubit it acts on.
                                // To avoid duplicates for multi-qubit gates, we can check first.
                                // if (qubitsToOps[lineageId].empty() || qubitsToOps[lineageId].back() != op) {
                                qubitsToOps[lineageId].push_back(op);
                                // if (isOneQubitOp(op)) {

                                // } else {

                                // }
                                // }

                                // Propagate the ID to the corresponding result, if it's also a qubit.
                                if (index < op->getNumResults()) {
                                    Value result = op->getResult(index);
                                    if (result.getType().isa<QubitType>()) {
                                        qubitsInitToOpRes[result] = lineageId;
                                    }
                                }
                            }
                        }
                    }
                }
            });
            
            AsmState state(module);
            for (auto &pair : qubitsToOps) {
                Value val = pair.first;
                auto &users = pair.second;

                std::string ssaName;
                llvm::raw_string_ostream stream(ssaName);
    
                val.printAsOperand(stream, state);

                llvm::outs() << "Lineage for " << ssaName << ":\n";
                for (auto *user : users) {
                    llvm::outs() << "    -> " << user->getName().getStringRef() << "\n";
                }
                llvm::outs() << "\n";
            }
        }
    }

} // namespace qoala::analysis::gatecount
