#include "Target/iQoala/iQoala.h"
#include "Analysis/Helpers/Helpers.h"

using namespace mlir;

#if __cplusplus >= 202002L
static std::string blockNameFmt = "b{}";
static std::string remoteIDFmt = "{}_id";
#else
static std::string blockNameFmt = "b%d";
static std::string remoteIDFmt = "%s_id";
#endif

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

    std::vector<RequestQuantumRoutine *> RequestSection::getRoutines() const {
        return this->routines;
    }

    void MetaSection::addParameter(const std::string &name) {
        this->globalParams.push_back(name);
    }

    void MetaSection::addRemote(const std::string &remoteName) {
        this->addParameter(helpers::formatString(remoteIDFmt, remoteName));
    }

    void MetaSection::addClassicalSocketForRemote(const std::string &remoteName, uint8_t socketID) {
        this->classicalSocketsMap.emplace(remoteName, socketID);

    }
    void MetaSection::addEPRSSocketForRemote(const std::string &remoteName, uint8_t socketID) {
        this->eprsSocketsMap.emplace(remoteName, socketID);
    }

    void MetaSection::setName(const std::string &programName) {
        this->name = programName;
    }

    Block *HostSection::createNewBlock() {
        auto *block = new Block();
        block->setName(helpers::formatString(blockNameFmt, this->hostBlocks.size()));
        this->hostBlocks.push_back(block);
        return block;
    }
}