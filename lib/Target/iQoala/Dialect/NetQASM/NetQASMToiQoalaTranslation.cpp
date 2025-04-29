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

#include <type_traits>

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

template<typename RoutineOp>
static LogicalResult convertRoutineOp(RoutineOp &op, ModuleTranslation *moduleTranslation) {
    static_assert(std::is_same_v<RoutineOp, LocalRoutineOp> || std::is_same_v<RoutineOp, RequestRoutineOp>);
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
        // Get the local routine of the operation under analysis
        std::string localRoutineName = qoala::dialects::helpers::getParentLocalRoutineName(op.getOperation());
        LocalQuantumRoutine *localRoutine = moduleTranslation->getQoalaModule()->getLocalRoutineByName(localRoutineName);

        if (localRoutine->getQubitNum(op.getOperand(i)) != 0xFF) {
            // returned qubit values must not be reported as return values
            continue;
        }

        // Get the register reference for the returned val
        Value returnedVal = op.getOperand(i);
        iQoalaRegReference *retValRegRef = moduleTranslation->getMappedRegRefForValue(returnedVal);
        assert(retValRegRef && "NetQASM return: trying to return a value that is not mapped!");
        iQoalaMCOperand *retValOperand = iQoalaMCOperand::createRegisterOperand(retValRegRef);
        assert(localRoutine && "NetQASM return: unknown local routine name!");

        // Add the name of the returned value to the local routine
        localRoutine->addReturnValue(qoala::helpers::formatString(returnNameFormat, i));

        // Create an immediate with the number of the returned value
        iQoalaMCOperand *immediateVal = iQoalaMCOperand::createImmediateOperand(i);

        // Add the store instruction to the routine
        const auto *storeInstr = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
            moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_STORE,
            {}, {}, {retValOperand, immediateVal},
            /*useOpOperands=*/false);

        if (!storeInstr) {
            return failure();
        }
    }
    (void) moduleTranslation->popFrame();
    return success();
}

template<typename RotationOp>
static iQoalaMCInstruction *createRotationInstr(RotationOp &op, ModuleTranslation *moduleTranslation, NetQASMMCInstr::OpCode opCode) {
    iQoalaRegReference *qbitReg = moduleTranslation->getMappedRegRefForValue(op.getQ());
    assert(qbitReg && "Create Rotation Instr: No mapped registry for qubit");
    const uint32_t nVal = op.getNVal().getLimitedValue(UINT32_MAX);
    const uint32_t expVal = op.getExpVal().getLimitedValue(UINT32_MAX);

    iQoalaMCOperand *qubitOperand = iQoalaMCOperand::createRegisterOperand(qbitReg);
    iQoalaMCOperand *nOperand = iQoalaMCOperand::createImmediateOperand(nVal);
    iQoalaMCOperand *expOperand = iQoalaMCOperand::createImmediateOperand(expVal);

    return qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
        moduleTranslation, op.getOperation(), opCode,
        {}, {}, {qubitOperand, nOperand, expOperand},
        /*useOpOperands=*/false
    );
}

static LogicalResult translateNetQASMOperation(Operation *operation, ModuleTranslation *moduleTranslation) {
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    const iQoalaModule *module = moduleTranslation->getQoalaModule();
    iQoalaContext *context = module->getiQoalaContext();
    // This is the main "dispatcher" for translating operations belonging to NetQASM dialect
    return llvm::TypeSwitch<Operation *, LogicalResult>(operation)
        .Case([&](QAllocOp op) -> LogicalResult {
            if (qoala::dialects::helpers::operationIsInsideRequestRoutineFunc(operation)) {
                // We need to report to the RequestRoutine object that we are creating 1 entangled pair
                const std::string reqRoutineName = qoala::dialects::helpers::getParentRequestRoutineName(operation);
                RequestQuantumRoutine *reqRoutine = module->getRequestRoutineByName(reqRoutineName);
                const uint32_t phyQubitID = context->allocateQubit();
                LLVM_DEBUG(llvm::dbgs() << "!!!!!!!!!!!!! Allocated qubit: " << phyQubitID << "\n");
                reqRoutine->registerQubit(op.getResult(), phyQubitID);
                reqRoutine->addVirtualIDArg(phyQubitID);
                // The registration of the remote (remoteID and eprsSocketID) will be done when
                // processing the eprs instruction.
                return success();
            }

            SmallVector<iQoalaMCOperand *> processedOperands;
            const uint32_t numQubit = context->allocateQubit();
            iQoalaMCOperand *immediateVal = iQoalaMCOperand::createImmediateOperand(numQubit);
            processedOperands.push_back(immediateVal);

            // Register the allocated qubit in the "uses" and (preemptively) in the "keep" section
            // of the local routine header
            const std::string localRoutineName = qoala::dialects::helpers::getParentLocalRoutineName(operation);
            LocalQuantumRoutine *quantumRoutine = module->getLocalRoutineByName(localRoutineName);
            assert(quantumRoutine && "NetQASM call: unknown local routine!");

            const auto *instruction = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_SET,
                {op.getResult()}, {Q}, processedOperands
                );
            quantumRoutine->registerQubit(op.getResult(), numQubit);
            return instruction ? success() : failure();
        })
        .Case([&](QFreeOp op) -> LogicalResult {
            // For the free operations, we simply register the qubit as not kept
            const std::string localRoutineName = qoala::dialects::helpers::getParentLocalRoutineName(operation);
            LocalQuantumRoutine *quantumRoutine = module->getLocalRoutineByName(localRoutineName);
            assert(quantumRoutine && "NetQASM qfree: unknown local routine!");

            const uint8_t qubitNum = quantumRoutine->releaseQubit(op.getQ());
            context->releaseQubit(qubitNum);
            return success();
        })
        .Case([&](QInitOp op) -> LogicalResult {
            const auto *instruction = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_INIT,
                {}, {});
            return instruction ? success() : failure();
        })
        .Case([&](LocalRoutineOp op) -> LogicalResult {
            LLVM_DEBUG(llvm::dbgs() << "Saw a local routine with name '" << op.getName() << "'\n");
            if (op.isExternal()) {
                // This case is, most likely, the "__qoala_convert_float_angle" builtin runtime function declaration
                return convertiQoalaRuntimeFunctionDeclaration(op);
            }
            return convertRoutineOp(op, moduleTranslation);
        })
        .Case([&](RequestRoutineOp op) -> LogicalResult {
            LLVM_DEBUG(llvm::dbgs() << "Saw a request routine with name '" << op.getName() << "'\n");
            return convertRoutineOp(op, moduleTranslation);
        })
        .Case([&](ReturnOp op) -> LogicalResult {
            if (qoala::dialects::helpers::operationIsInsideRequestRoutineFunc(operation)) {
                // We don't need to check the validity of the return operation
                // we will simply assume that it was validated in the lowering conversion passes
                // Being this said we will test the first return type. If it is i1, then we
                // are returning the measurement of a qubit, hence the type of the request operation
                // must be changed to "create_measure"
                const std::string reqRoutineName = qoala::dialects::helpers::getParentRequestRoutineName(operation);
                RequestQuantumRoutine *reqRoutine = module->getRequestRoutineByName(reqRoutineName);
                if (operation->getOperandTypes()[0].isInteger(1)) {
                    reqRoutine->changeReqTypeToMeasure();
                }
                for (uint32_t i = 0; i < op.getNumOperands(); i++) {
                    // We also need to report the returned variable name
                    if (reqRoutine->getQubitNum(op.getOperand(i)) == 0xFF) {
                        reqRoutine->addReturnValue(qoala::helpers::formatString(returnNameFormat, i));
                    }
                }
                moduleTranslation->popFrame();
                return success();
            }
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
        .Case([&](HadamardOp op) -> LogicalResult {
            const auto *instruction = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_H,
                {}, {}
                );
            return instruction ? success() : failure();
        })
        .Case([&](CnotOp op) -> LogicalResult {
            const auto *instruction = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_CNOT,
                {}, {}
                );
            return instruction ? success() : failure();
        })
        .Case([&](CzOp op) -> LogicalResult {
            const auto *instruction = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_CPHASE,
                {}, {}
                );
            return instruction ? success() : failure();
        })
        .Case([&](CrotXOp op) -> LogicalResult {
            operation->emitError("NetQASM crot_x: Operation not supported.");
            return failure();
        })
        .Case([&](MeasureOp op) -> LogicalResult {
            if (qoala::dialects::helpers::operationIsInsideRequestRoutineFunc(operation)) {
                // Nothing to do here: we simply measure the qubit whose result will be returned later
                return success();
            }
            // Since measurements release the qubit, we need to register the qubit as not kept
            const std::string localRoutineName = qoala::dialects::helpers::getParentLocalRoutineName(op.getOperation());
            LocalQuantumRoutine *quantumRoutine = module->getLocalRoutineByName(localRoutineName);
            assert(quantumRoutine && "NetQASM measure: unknown local routine!");

            const uint8_t qubitNum = quantumRoutine->releaseQubit(op.getQ());
            context->releaseQubit(qubitNum);
            const auto *instruction = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_MEAS,
                {op.getResult()}, {M}
                );
            return instruction ? success() : failure();
        })
        .Case([&](EprsOp op) -> LogicalResult {
            const std::string reqRoutineName = qoala::dialects::helpers::getParentRequestRoutineName(operation);
            RequestQuantumRoutine *reqRoutine = module->getRequestRoutineByName(reqRoutineName);
            // Mark the allocated qubit as entangled
            reqRoutine->addEntangledQubitID(reqRoutine->getQubitNum(op.getQ()));
            // The registration of the remote (remoteID and eprsSocketID)
            // Search for the Remote name and its eprsSocketID in the module
            const StringRef remoteName = op.getRemoteAttr().getValue();
            const uint8_t eprsSocketID = module->getEPRSSocketIDForRemote(remoteName);
            const std::string remoteParamName = module->getParamNameForRemote(remoteName.str());
            reqRoutine->reportRemote(remoteParamName, eprsSocketID);
            return success();
        })
        .Case([](EprsMeasureOp op) -> LogicalResult {
            // Nothing to do here: we simply measure the qubit whose result will be returned later
            return success();
        })
        .Default([](Operation *op) -> LogicalResult {
            return op->emitOpError("Unknown way to translate a NetQASM operation to iQoala'");
        });
}

namespace qoala::translate {
    LogicalResult NetQASMToiQoalaTranslation::convertOperation(Operation *op, ModuleTranslation *moduleTranslation) const {
        return translateNetQASMOperation(op, moduleTranslation);
    }
}