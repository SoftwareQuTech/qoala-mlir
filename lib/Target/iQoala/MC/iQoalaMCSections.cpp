#include "Analysis/Helpers/Helpers.h"
#include "Target/iQoala/iQoala.h"

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

    void NetQASMSection::addRoutine(LocalQuantumRoutine *routine) { this->routines.push_back(routine); }

    std::vector<LocalQuantumRoutine *> NetQASMSection::getRoutines() const { return this->routines; }

    void RequestSection::addRoutine(RequestQuantumRoutine *routine) { this->routines.push_back(routine); }

    std::vector<RequestQuantumRoutine *> RequestSection::getRoutines() const { return this->routines; }

    void MetaSection::addParameter(const std::string &name) { this->globalParams.push_back(name); }

    void MetaSection::addRemote(const std::string &remoteName) {
        const std::string remoteParamName = helpers::formatString(remoteIDFmt, remoteName.c_str());
        this->addParameter(remoteParamName);
        this->remoteParamNames.emplace(remoteName, remoteParamName);
    }

    void MetaSection::addClassicalSocketForRemote(const std::string &remoteName, uint8_t socketID) {
        this->classicalSocketsMap.emplace(remoteName, socketID);
    }

    void MetaSection::addEPRSSocketForRemote(const std::string &remoteName, uint8_t socketID) {
        this->eprsSocketsMap.emplace(remoteName, socketID);
    }

    uint8_t MetaSection::getClassicalSocketForRemote(const std::string &remoteName) const {
        return static_cast<uint8_t>(this->classicalSocketsMap.at(remoteName));
    }

    uint8_t MetaSection::getEPRSSocketForRemote(const std::string &remoteName) const {
        return static_cast<uint8_t>(this->eprsSocketsMap.at(remoteName));
    }

    std::string MetaSection::getParamNameForRemote(const std::string &remoteName) const {
        return this->remoteParamNames.at(remoteName);
    }

    void MetaSection::setName(const std::string &programName) { this->name = programName; }

    Block *HostSection::createNewBlock() {
        auto *block = new Block();
        block->setName(helpers::formatString(blockNameFmt, this->hostBlocks.size()));
        this->hostBlocks.push_back(block);
        return block;
    }

    void HostSection::deleteEmptyBlocks() {
        std::vector<Block *> blocksCpy;
        for (Block *block : this->hostBlocks) {
            if (!block->isEmpty()) {
                blocksCpy.push_back(block);
            } else {
                delete block;
            }
        }
        this->hostBlocks = blocksCpy;
    }

    LogicalResult HostSection::setBlockTypes() const {
        // This logic is based on the criteria explained on ticket #73
        for (Block *block : this->hostBlocks) {
            if (block->blockContainsRunRequest()) {
                if (block->getNumInstructions() != 1) {
                    return failure();
                }
                block->setType(Block::QC);
                continue;
            }
            if (block->blockContainsRunSubRoutine()) {
                if (block->getNumInstructions() != 1) {
                    return failure();
                }
                block->setType(Block::QL);
                continue;
            }
            if (block->blockContainsRecvMsg()) {
                if (block->getNumInstructions() != 1) {
                    return failure();
                }
                block->setType(Block::CC);
            }
            // By default, block are CL type, so if none of the cases
            // above matched, we leave the block as is.
        }
        return success();
    }

} // namespace qoala::iqoala
