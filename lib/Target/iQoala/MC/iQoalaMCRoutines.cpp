#include "Target/iQoala/iQoala.h"
#include "Analysis/Helpers/Helpers.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "iQoalaMCRoutines"

using namespace mlir;

namespace qoala::iqoala {
    void QuantumRoutine::registerQubit(const Value &value, const uint8_t phyQubitNum) {
        const auto result = this->qubitMap.try_emplace(value, phyQubitNum);
        (void) result;
        assert(result.second && "Attempting to map a qubit value that has already been mapped");
    }

    uint8_t QuantumRoutine::releaseQubit(const Value &value) {
        uint8_t phyQubitNum = 0xFF;
        if (this->qubitMap.contains(value)) {
            phyQubitNum = this->qubitMap.at(value);
            this->qubitMap.erase(value);
        }
        return phyQubitNum;
    }

    uint8_t QuantumRoutine::getQubitNum(const Value &value) const {
        if (this->qubitMap.contains(value)) {
            return this->qubitMap.at(value);
        }
        return 0xFF;
    }

    LocalQuantumRoutine *LocalQuantumRoutine::createLocalRoutine(const StringRef name) {
        return new LocalQuantumRoutine(name);
    }

    RequestQuantumRoutine *RequestQuantumRoutine::createRequestRoutine(const StringRef name) {
        return new RequestQuantumRoutine(name);
    }

    void LocalQuantumRoutine::addInstruction(assembly::NetQASMMCInstr *instruction) {
        this->instructions.push_back(instruction);
    }

    void LocalQuantumRoutine::addArgument(const std::string &argName) {
        this->params.push_back(argName);
    }

    void LocalQuantumRoutine::registerQubit(const Value &value, const uint8_t phyQubitNum) {
        this->QuantumRoutine::registerQubit(value, phyQubitNum);

        this->usesQubits.emplace(phyQubitNum);
        // We eagerly mark the qubit as "keeps"... only if we qfree or measure it we delete it from the "keeps".
        this->keepsQubits.emplace(phyQubitNum);
    }

    uint8_t LocalQuantumRoutine::releaseQubit(const Value &value) {
        const uint8_t phyQubitNum = this->QuantumRoutine::releaseQubit(value);
        if (this->keepsQubits.find(phyQubitNum) != this->keepsQubits.end()) {
            this->keepsQubits.erase(phyQubitNum);
        }
        return phyQubitNum;
    }

    uint8_t LocalQuantumRoutine::getQubitNum(const Value &value) const {
        return QuantumRoutine::getQubitNum(value);
    }

    void LocalQuantumRoutine::addReturnValue(const std::string &valName) {
        this->returns.push_back(valName);
    }

    void LocalQuantumRoutine::resolveInternalInstrRefs() const {
        DenseMap<Operation *, int32_t> operationToIndex;
        DenseMap<assembly::iQoalaMCExpr *, Operation *> expressionsToResolve;

        for (uint32_t i = 0; i < this->instructions.size(); i++) {
            const auto *instruction = this->instructions[i];
            Operation *opPtr = instruction->getOriginalOp();
            // Hack to correctly compute branching displacements despite some MLIR ops maps
            // to multiple NetQASM instructions. If this is the case, all the mapped instructions
            // will share the same instruction->getOriginalOp() attribute. Since this is a
            // problem for the operationToIndex map (the originalOp pointers are used as keys)
            // we need to artificially "fix" duplicated key issue, by creating a "unique" key for
            // the extra NetQASM instructions inserted.
            if (operationToIndex.contains(instruction->getOriginalOp())) {
                // This is an "introduced" instruction, we artificially create an op index pointer
                // so we compute the jump displacements correctly
                // With this hack, the very first operation that was mapped (i.e. the original MLIR
                // operation) would be the instruction to jump to, if referenced by some branching
                // instruction
                opPtr += 4 * i;
            }
            LLVM_DEBUG(llvm::dbgs() << "+++ Emplacing ptr: '" << opPtr << "', i = " << i
                << ", opcode = " << instruction->getOpcode() << ", op = " << instruction <<"\n");
            const auto result = operationToIndex.try_emplace(opPtr, i);
            (void) result;
            assert(result.second && "Resolve Instr Refs: Op index pointer already present!");

            for (const auto *param : instruction->getOperands()) {
                if (param->isExpression() && param->getExpression()->isInstructionRef()) {
                    expressionsToResolve.try_emplace(param->getExpression(), opPtr);
                }
            }
        }

        for (auto exprToResolve : expressionsToResolve) {
            assert(operationToIndex.contains(exprToResolve.second) &&
                "Resolve Instr Refs: Instruction containing an InstrRef comes from an MLIR operation not present in NetQASM body!");
            assert(operationToIndex.contains(exprToResolve.first->getTargetOp()) &&
                "Resolve Instr Refs: InstrRef refers to an operation not found within the NetQASM body!");

            const int32_t sourceIndex = operationToIndex[exprToResolve.first->getTargetOp()];
            const int32_t targetIndex = operationToIndex[exprToResolve.second];

            exprToResolve.first->resolveDisplacement(sourceIndex - targetIndex);
        }
    }

    void LocalQuantumRoutine::finalizeRoutine() {
        // Nothing to do here
    }

    void VirtualIDs::addArg(const uint32_t arg) {
        this->args.push_back(arg);
    }

    void VirtualIDs::setType(const VirtualIDType type) {
        this->type = type;
    }

    void VirtualIDs::resolve() {
        if (this->args.size() == 1) {
            this->type = ALL;
            return;
        }

        for (uint32_t i = 1; i < this->args.size(); i++) {
            if (this->args[i-1] + 1 != this->args[i]) {
                this->type = CUSTOM;
            }
        }
        this->type = INCREMENT;
    }


    void RequestQuantumRoutine::addEntangledQubitID(const uint32_t phyQubitID) {
        this->entangledQubitsIDs.push_back(phyQubitID);
    }

    void RequestQuantumRoutine::addReturnValue(const std::string &valName) {
        this->returns.push_back(valName);
    }

    void RequestQuantumRoutine::reportRemote(const std::string &remoteID, const uint8_t eprSocketID) {
        this->remoteID = remoteID;
        this->eprSocketID = eprSocketID;
    }

    void RequestQuantumRoutine::changeReqTypeToMeasure() {
        this->type = MEASURE_DIRECTLY;
    }

    void RequestQuantumRoutine::changeReqTypeToRSP() {
        this->type = RSP;
    }

    void RequestQuantumRoutine::addVirtualIDArg(const uint32_t virtualID) {
        this->virtualIDs.addArg(virtualID);
    }

    void RequestQuantumRoutine::addArgument(const std::string &argName) {
        // TODO
    }

    void RequestQuantumRoutine::addInstruction(assembly::NetQASMMCInstr *instruction) {
        // Since request routines do not have attached instructions,
        // there is literally nothing to do here.
    }

    void RequestQuantumRoutine::registerQubit(const Value &value, uint8_t phyQubitNum) {
        this->QuantumRoutine::registerQubit(value, phyQubitNum);
    }

    uint8_t RequestQuantumRoutine::releaseQubit(const Value &value) {
        return this->QuantumRoutine::releaseQubit(value);
    }

    [[nodiscard]]
    uint8_t RequestQuantumRoutine::getQubitNum(const Value &value) const {
        return QuantumRoutine::getQubitNum(value);
    }

    void RequestQuantumRoutine::finalizeRoutine() {
        // We resolve the virtual IDs method depending on how thw qubit IDs were assigned
        this->virtualIDs.resolve();
    }

    raw_ostream &operator<<(raw_ostream &os, const RequestQuantumRoutine::RequestCallback requestCallback) {
        switch (requestCallback) {
            case RequestQuantumRoutine::SEQUENTIAL:
                 os << "sequential";
                break;
            case RequestQuantumRoutine::WAIT_ALL:
                os << "wait_all";
                break;
        }
        return os;
    }

    raw_ostream &operator<<(raw_ostream &os, const VirtualIDs &virtualIDs) {
        switch (virtualIDs.type) {
            case VirtualIDs::VirtualIDType::ALL:
                os << "all " << (virtualIDs.args.empty() ? 0 : *std::min_element(virtualIDs.args.begin(), virtualIDs.args.end()));
                break;
            case VirtualIDs::VirtualIDType::INCREMENT:
                os << "increment " << (virtualIDs.args.empty() ? 0 : *std::min_element(virtualIDs.args.begin(), virtualIDs.args.end()));
                break;
            case VirtualIDs::VirtualIDType::CUSTOM:
                os << "custom " << helpers::formatVector(virtualIDs.args);
                break;
        }
        return os;
    }

    raw_ostream &operator<<(raw_ostream &os, const RequestQuantumRoutine::RequestType requestType) {
        switch (requestType) {
            case RequestQuantumRoutine::CREATE_KEEP:
                os << "create_keep";
                break;
            case RequestQuantumRoutine::MEASURE_DIRECTLY:
                os << "measure_directly";
                break;
            case RequestQuantumRoutine::RSP:
                os << "rsp";
                break;
        }
        return os;
    }


    raw_ostream &operator<<(raw_ostream &os, const RequestQuantumRoutine::RequestRole requestRole) {
        switch (requestRole) {
            case RequestQuantumRoutine::CREATE:
                os << "create";
                break;
            case RequestQuantumRoutine::RECEIVE:
                os << "receive";
                break;
        }
        return os;
    }

    void LocalQuantumRoutine::print(raw_ostream &os) const {
        os << "SUBROUTINE " << this->name << "\n";
        os << "params: " << helpers::formatVector(this->params) << "\n";
        os << "returns: " << helpers::formatVector(this->returns) << "\n";
        os << "uses: " << helpers::formatSet(this->usesQubits) << "\n";
        os << "keeps: " << helpers::formatSet(this->keepsQubits) << "\n";

        os << "NETQASM_START\n";
        for (const assembly::iQoalaMCInstruction *instruction : this->instructions) {
            os << tabStr << *instruction << "\n";
        }
        os << "NETQASM_END\n";
    }

    void RequestQuantumRoutine::print(raw_ostream &os) const {
        os << "REQUEST " << this->name << "\n";
        os << "callback_type: " << this->requestCallback << "\n";
        os << "callback: " << (this->callback ? this->callback->getName() : "") << "\n";
        os << "return_vars: " << helpers::formatVector(this->returns) << "\n";
        os << "remote_id: " << "{" << this->remoteID << "}" << "\n";
        os << "epr_socket_id: " << this->eprSocketID << "\n";
        os << "num_pairs: " << this->entangledQubitsIDs.size() << "\n";
        os << "virt_ids: " << this->virtualIDs << "\n";
        os << "timeout: " << 1000 << "\n"; // This field is not described in the paper
        os << "fidelity: " << this->fidelity << "\n";
        os << "type: " << this->type << "\n";
        os << "role: " << this->requestRole << "\n";
    }
}