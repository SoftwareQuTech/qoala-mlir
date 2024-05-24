#include "mlir/IR/Operation.h"
#include "Target/iQoala/ModuleTranslation.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "Target/iQoala/Dialect/NetQASM/NetQASMToiQoalaTranslation.h"
#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Debug.h"

#include "Dialect/NetQASM/NetQASM.h"

#define DEBUG_TYPE "netqasm-translation"

using namespace qoala::dialects::netqasm;
using namespace qoala::iqoala;

static LogicalResult convertLocalRoutineOp(LocalRoutineOp &op, ModuleTranslation &moduleTranslation) {
    for (Operation &operation : op.getBody().getOps()) {
        if (failed(moduleTranslation.convertOperation(operation))) {
            return operation.emitOpError("cannot convert the operation '") << operation << "'\n";
        }
    }
    return success();
}

static LogicalResult convertiQoalaRuntimeFunctionDeclaration(LocalRoutineOp &op) {
    return success();
}

static LogicalResult translateNetQASMOperation(Operation *operation, ModuleTranslation &moduleTranslation) {
    // TODO - Implement this dispatcher
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    // Use this example for applying different behavior depending on the type of the operation under analysis
    // This is the main "dispatcher" for translating operations belonging yo NetQASM dialect
    return llvm::TypeSwitch<Operation *, LogicalResult>(operation)
            .Case([](QAllocOp op) -> LogicalResult {
                return success();
            })
            .Case([](QInitOp op) -> LogicalResult {
                return success();
            })
            .Case([&](LocalRoutineOp op) -> LogicalResult {
                LLVM_DEBUG(llvm::dbgs() << "Saw a local routine with name '" << op.getName() << "'\n");
                if (op.isExternal()) {
                    return convertiQoalaRuntimeFunctionDeclaration(op);
                } else {
                    return convertLocalRoutineOp(op, moduleTranslation);
                }
            })
            .Case([](RequestRoutineOp op) -> LogicalResult {
                LLVM_DEBUG(llvm::dbgs() << "Saw a request routine with name '" << op.getName() << "'\n");
                return success();
            })
            .Case([](ReturnOp op) -> LogicalResult {
                return success();
            })
            .Case([](RotateXOp op) -> LogicalResult {
                return success();
            })
            .Case([](RotateYOp op) -> LogicalResult {
                return success();
            })
            .Case([](RotateZOp op) -> LogicalResult {
                return success();
            })
            .Case([](HadamardOp op) -> LogicalResult {
                return success();
            })
            .Case([](CnotOp op) -> LogicalResult {
                return success();
            })
            .Case([](CzOp op) -> LogicalResult {
                return success();
            })
            .Case([](CrotXOp op) -> LogicalResult {
                return success();
            })
            .Case([](MeasureOp op) -> LogicalResult {
                return success();
            })
            .Case([](EprsOp op) -> LogicalResult {
                return success();
            })
            .Case([](EprsMeasureOp op) -> LogicalResult {
                return success();
            })
            .Default([](Operation *op) -> LogicalResult {
                return op->emitOpError("Unknown way to translate a NetQASM operation to iQoala: '") << *op << "'\n";
            });
}

namespace qoala::translate {
    class NetQASMToiQoalaTranslationInterface : public QoalaTranslationDialectInterface {
    public:
        using QoalaTranslationDialectInterface::QoalaTranslationDialectInterface;
        LogicalResult convertOperation(Operation *op, ModuleTranslation &moduleTranslation) const final {
            return translateNetQASMOperation(op, moduleTranslation);
        }

    };

    void registerNetQASMToiQoalaTranslations(DialectRegistry &registry) {
        registry.insert<NetQASMDialect>();
        registry.addExtension(+[](MLIRContext *ctx, NetQASMDialect *dialect) {
            dialect->addInterfaces<NetQASMToiQoalaTranslationInterface>();
        });
    }
}