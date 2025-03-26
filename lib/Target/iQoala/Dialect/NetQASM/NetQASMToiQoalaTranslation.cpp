#include "mlir/IR/Operation.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Analysis/Helpers/Helpers.h"
#include "Conversion/Helpers/Helpers.h"
#include "Dialect/Helpers/DialectHelpers.h"
#include "Target/iQoala/ModuleTranslation.h"
#include "Target/iQoala/MC/Helpers.h"
#include "Target/iQoala/Dialect/NetQASM/NetQASMToiQoalaTranslation.h"

#include "Dialect/QNet/Passes.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "netqasm-translation"

using namespace mlir;
using namespace qoala::assembly;
using namespace qoala::translate;
using namespace qoala::dialects;
using namespace qoala::iqoala;
using namespace qoala::dialects::netqasm;

#if  __cplusplus >= 202002L
/* When using C++20 or newer standard, the format must be "python-style" */
static const std::string returnNameFormat = "m{}";
#else
/* When using C++17 or older standard, the format must be "C-style" */
static const std::string returnNameFormat = "m%d";
#endif

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
        localRoutine->addReturnValue(qoala::helpers::formatString(returnNameFormat, i));

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

template<typename RotationOp>
static iQoalaMCInstruction *createRotationInstr(RotationOp &op, ModuleTranslation *moduleTranslation, NetQASMMCInstr::OpCode opCode) {
    iQoalaRegReference *qbitReg = moduleTranslation->getMappedRegReference(op.getQ());
    assert(qbitReg && "Create Rotation Instr: No mapped registry for qubit");
    const uint32_t nVal = op.getNVal().getLimitedValue(UINT32_MAX);
    const uint32_t expVal = op.getExpVal().getLimitedValue(UINT32_MAX);

    iQoalaMCOperand *qubitOperand = iQoalaMCOperand::createRegisterOperand(qbitReg);
    iQoalaMCOperand *nOperand = iQoalaMCOperand::createImmediateOperand(nVal);
    iQoalaMCOperand *expOperand = iQoalaMCOperand::createImmediateOperand(expVal);

    return qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
        moduleTranslation, op.getOperation(), opCode,
        std::nullopt, std::nullopt, {qubitOperand, nOperand, expOperand},
        /*useOpOperands=*/false
    );
}

static LogicalResult translateNetQASMOperation(Operation *operation, ModuleTranslation *moduleTranslation) {
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    // This is the main "dispatcher" for translating operations belonging to NetQASM dialect
    return llvm::TypeSwitch<Operation *, LogicalResult>(operation)
        .Case([&](QAllocOp op) -> LogicalResult {
            SmallVector<iQoalaMCOperand *> processedOperands;
            const uint32_t numQubit = moduleTranslation->getQoalaModule()->getiQoalaContext()->allocateQubit();
            iQoalaMCOperand *immediateVal = iQoalaMCOperand::createImmediateOperand(numQubit);
            processedOperands.push_back(immediateVal);

            // Register the allocated qubit in the "uses" and (preemptively) in the "keep" section
            // of the local routine header
            const std::string localRoutineName = qoala::dialects::helpers::getParentNetQASMRoutineName(op.getOperation());
            LocalQuantumRoutine *quantumRoutine = moduleTranslation->getQoalaModule()->getLocalRoutineByName(localRoutineName);
            assert(quantumRoutine && "NetQASM call: unknown local routine!");

            const auto *instruction = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_SET,
                op.getResult(), Q, processedOperands
                );
            quantumRoutine->registerQubit(op.getResult(), numQubit);
            return instruction ? success() : failure();
        }).Case([&](QFreeOp op) -> LogicalResult {
            // For the free operations, we simply register the qubit as not kept
            const std::string localRoutineName = qoala::dialects::helpers::getParentNetQASMRoutineName(op.getOperation());
            LocalQuantumRoutine *quantumRoutine = moduleTranslation->getQoalaModule()->getLocalRoutineByName(localRoutineName);
            assert(quantumRoutine && "NetQASM qfree: unknown local routine!");

            quantumRoutine->releaseQubit(op.getQ());
            return success();
        })
        .Case([&](QInitOp op) -> LogicalResult {
            const auto *instruction = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_INIT,
                std::nullopt, std::nullopt, {}
                );
            return instruction ? success() : failure();
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
        .Case([&](RotateXOp op) -> LogicalResult {
            return createRotationInstr(op, moduleTranslation, NetQASMMCInstr::OP_ROT_X) ? success() : failure();
        })
        .Case([&](RotateYOp op) -> LogicalResult {
            return createRotationInstr(op, moduleTranslation, NetQASMMCInstr::OP_ROT_Y) ? success() : failure();
        })
        .Case([&](RotateZOp op) -> LogicalResult {
            return createRotationInstr(op, moduleTranslation, NetQASMMCInstr::OP_ROT_Z) ? success() : failure();
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
        .Case([&](MeasureOp op) -> LogicalResult {
            // Since measurements release the qubit, we need to register the qubit as not kept
            const std::string localRoutineName = qoala::dialects::helpers::getParentNetQASMRoutineName(op.getOperation());
            LocalQuantumRoutine *quantumRoutine = moduleTranslation->getQoalaModule()->getLocalRoutineByName(localRoutineName);
            assert(quantumRoutine && "NetQASM measure: unknown local routine!");

            quantumRoutine->releaseQubit(op.getQ());
            const auto *instruction = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_MEAS,
                op.getResult(), M, {}
                );
            return instruction ? success() : failure();
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