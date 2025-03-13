#include "mlir/IR/Operation.h"
#include "llvm/ADT/TypeSwitch.h"
#include "Target/iQoala/ModuleTranslation.h"
#include "Target/iQoala/MC/Helpers.h"
#include "Conversion/Helpers/Helpers.h"
#include "Dialect/Helpers/DialectHelpers.h"
#include "Target/iQoala/Dialect/NetQASM/NetQASMToiQoalaTranslation.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "netqasm-translation"

using namespace mlir;
using namespace qoala::assembly;
using namespace qoala::translate;
using namespace qoala::dialects;
using namespace qoala::iqoala;
using namespace qoala::dialects::netqasm;

static LogicalResult convertLocalRoutineOp(LocalRoutineOp &op, ModuleTranslation *moduleTranslation) {
    for (Operation &operation : op.getBody().getOps()) {
        if (failed(moduleTranslation->convertOperation(operation))) {
            return operation.emitOpError("cannot convert the operation '") << operation << "'\n";
        }
    }
    return success();
}

static LogicalResult convertiQoalaRuntimeFunctionDeclaration(LocalRoutineOp &op) {
    assert(op.getName() == qoala::helpers::angle::angleConversionFunctionName);
    // We don't need to do anything specific here; simply "ignore" this declaration, since
    // the definition will be provided by the runtime
    return success();
}

static LogicalResult processReturnOp(ModuleTranslation *moduleTranslation, ReturnOp &op) {
    // Counter-intuitive: the returned values are the *operands* of the return op
    for (uint32_t i = 0; i < op.getNumOperands(); i++) {
        // Get the register reference for the returned val
        Value returnedVal = op.getOperand(i);
        iQoalaRegReference *retValRegRef = moduleTranslation->getMappedRegReference(returnedVal);
        assert(retValRegRef && "NetQASM return: trying to return a value that is not mapped!");
        iQoalaMCOperand *retValOperand = iQoalaMCOperand::createRegisterOperand(retValRegRef);

        // And the local routine of the operation under analysis
        std::string localRoutineName = qoala::dialects::helpers::getParentNetQASMRoutineName(op.getOperation());
        LocalQuantumRoutine *localRoutine = moduleTranslation->getQoalaModule()->getLocalRoutineByName(localRoutineName);
        assert(localRoutine && "NetQASM return: unknown local routine name!");

        // Add the name of the returned value to the local routine
        // std::format was introduced as part of C++20 standard. We will use the old way
        // to format a string. More info on function `getNewFunctionName` in the file
        // `lib/Analysis/QMem/Functionize.cpp`.
        const auto nameFormat = "m%d";
        const int length = std::snprintf(nullptr, 0, nameFormat, i);
        std::vector<char> returnName(length + 1);
        std::sprintf(returnName.data(), nameFormat, i);
        localRoutine->addReturnValue(returnName.data());

        // Create an immediate with the number of the returned value
        iQoalaMCOperand *immediateVal = iQoalaMCOperand::createImmediateOperand(i);

        // Add the store instruction to the routine
        auto *storeInstr = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
            moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_STORE,
            std::nullopt, std::nullopt, {retValOperand, immediateVal},
            /*useOpOperands=*/false);

        if (!storeInstr) {
            return failure();
        }
    }
    return success();
}

static LogicalResult translateNetQASMOperation(Operation *operation, ModuleTranslation *moduleTranslation) {
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
            .Case([&](ReturnOp op) -> LogicalResult {
                return processReturnOp(moduleTranslation, op);
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