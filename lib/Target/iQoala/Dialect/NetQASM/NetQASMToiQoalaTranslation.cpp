#include "mlir/IR/Operation.h"
#include "Target/iQoala/ModuleTranslation.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "Target/iQoala/Dialect/NetQASM/NetQASMToiQoalaTranslation.h"
#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "netqasm-translation"

using namespace mlir;
using namespace qoala::dialects::netqasm;

static LogicalResult convertLocalRoutineOp(LocalRoutineOp &op, qoala::translate::ModuleTranslation *moduleTranslation) {
    for (Operation &operation : op.getBody().getOps()) {
        if (failed(moduleTranslation->convertOperation(operation))) {
            return operation.emitOpError("cannot convert the operation '") << operation << "'\n";
        }
    }
    return success();
}

static LogicalResult convertiQoalaRuntimeFunctionDeclaration(LocalRoutineOp &op) {
    assert(op.getName() == "__qoala_convert_float_angle");
    // We don't need to do anything specific here; simply "ignore" this declaration, since
    // the definition will be provided by the runtime
    return success();
}

static LogicalResult translateNetQASMOperation(Operation *operation, qoala::translate::ModuleTranslation *moduleTranslation) {
    // TODO - Implement this dispatcher
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    // Use this example for applying different behavior depending on the type of the operation under analysis
    // This is the main "dispatcher" for translating operations belonging to NetQASM dialect
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
                    // This case is, most likely, the "__qoala_convert_float_angle" builtin runtime function declaration
                    return convertiQoalaRuntimeFunctionDeclaration(op);
                }
                return convertLocalRoutineOp(op, moduleTranslation);
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
    LogicalResult NetQASMToiQoalaTranslation::convertOperation(Operation *op, ModuleTranslation *moduleTranslation) const {
        return translateNetQASMOperation(op, moduleTranslation);
    }
}