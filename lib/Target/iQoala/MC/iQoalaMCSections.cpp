#include "Target/iQoala/iQoala.h"
#include "Analysis/Helpers/Helpers.h"

using namespace mlir;

namespace qoala::iqoala {
    void MetaSection::print(raw_ostream &os) const {
        os << "META START\n";
        os << tabStr << "name: " << this->name << "\n";
        os << tabStr << "parameters: " << helpers::formatVector(this->globalParams) << "\n";
        os << tabStr << "csockets: " << helpers::formatMap(this->classicalSocketsMap) << "\n";
        os << tabStr << "epr_sockets: " << helpers::formatMap(this->eprsSocketsMap) << "\n";
        os << "META END";
    }

    void HostSection::print(raw_ostream &os) const {
        // Iteratively print all the blocks
        for (const Block *block : this->hostBlocks) {
            os << *block << "\n";
        }
    }

    void NetQASMSection::print(raw_ostream &os) const {
        // Iteratively print all the routines:
        for (const QuantumRoutine *routine : this->routines) {
            os << *routine << "\n";
        }
    }

    void RequestSection::print(raw_ostream &os) const {
        // Iteratively print all the routines:
        for (const QuantumRoutine *routine : this->routines) {
            os << *routine << "\n";
        }
    }

    void NetQASMSection::addRoutine(LocalQuantumRoutine *routine) {
        this->routines.push_back(routine);
    }

    std::vector<LocalQuantumRoutine *> NetQASMSection::getRoutines() const {
        return this->routines;
    }

    void RequestSection::addRoutine(RequestQuantumRoutine *routine) {
        this->routines.push_back(routine);
    }

    void MetaSection::addRemote(const std::string &remoteName) {
        this->globalParams.push_back(remoteName);
        // TODO - We assume that all declared remotes are connected using
        //  both classical and epr sockets
        this->classicalSocketsMap.insert(std::pair{remoteName, 0});
        this->eprsSocketsMap.insert(std::pair{remoteName, 0});
    }

    void MetaSection::setName(const std::string &programName) {
        this->name = programName;
    }

    Block *HostSection::createNewBlock() {
        auto *block = new Block();
        this->hostBlocks.push_back(block);
        return block;
    }
}