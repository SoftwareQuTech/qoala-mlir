#include "Target/iQoala/iQoala.h"
#include "Analysis/Helpers/Helpers.h"

using namespace mlir;

namespace qoala::iqoala {
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

    void LocalQuantumRoutine::addReturnValue(const std::string &valName) {
        this->returns.push_back(valName);
    }
    void LocalQuantumRoutine::resolveInternalInstrRefs() const {
        DenseMap<Operation *, uint32_t> operationToIndex;
        DenseMap<assembly::iQoalaMCExpr *, Operation *> expressionsToResolve;

        for (uint32_t i = 0; i < this->instructions.size(); i++) {
            const auto *instruction = this->instructions[i];
            for (const auto *param : instruction->getOperands()) {
                operationToIndex.try_emplace(instruction->getOriginalOp(), i);
                if (param->isExpression() && param->getExpression()->isInstructionRef()) {
                    expressionsToResolve.try_emplace(param->getExpression(), instruction->getOriginalOp());
                }
            }
        }

        for (auto exprToResolve : expressionsToResolve) {
            assert(operationToIndex.contains(exprToResolve.second) &&
                "Resolve Instr Refs: Instruction containing an InstrRef comes from an MLIR operation not present in NetQASM body!");
            assert(operationToIndex.contains(exprToResolve.first->getTargetOp()) &&
                "Resolve Instr Refs: InstrRef refers to an operation not found within the NetQASM body!");

            const uint32_t sourceIndex = operationToIndex[exprToResolve.first->getTargetOp()];
            const uint32_t targetIndex = operationToIndex[exprToResolve.second];

            exprToResolve.first->resolveDisplacement(sourceIndex - targetIndex);
        }
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
                os << "all " << virtualIDs.args[0];
                break;
            case VirtualIDs::VirtualIDType::INCREMENT:
                os << "increment " << virtualIDs.args[0];
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
        os << "uses: " << helpers::formatVector(this->usesQubits) << "\n";
        os << "keeps: " << helpers::formatVector(this->keepsQubits) << "\n";

        os << "NETQASM_START\n";
        for (const assembly::NetQASMMCInstr *instruction : this->instructions) {
            os << tabStr << *instruction << "\n";
        }
        os << "NETQASM_END\n";
    }

    void RequestQuantumRoutine::print(raw_ostream &os) const {
        os << "REQUEST " << this->name << "\n";
        os << "callback_type: " << this->requestCallback << "\n";
        os << "callback: " << this->callback->getName() << "\n";
        os << "return_vars: " << helpers::formatVector(this->returns) << "\n";
        os << "remote_id: " << "{" << this->remoteID << "}" << "\n";
        os << "epr_socket_id: " << this->eprSocketID << "\n";
        os << "num_pairs: " << this->numPairs << "\n";
        os << "virt_ids: " << this->virtualIDs << "\n";
        os << "timeout: " << 1000 << "\n"; // This field is not described in the paper
        os << "fidelity: " << this->fidelity << "\n";
        os << "typ: " << this->type << "\n";
        os << "role: " << this->requestRole << "\n";

        os << "NETQASM_START\n";
        for (const assembly::NetQASMMCInstr *instruction : this->instructions) {
            os << tabStr << *instruction << "\n";
        }
        os << "NETQASM_END\n";
    }
}