#include "Target/iQoala/iQoala.h"
#include "sstream"

namespace qoala::iqoala {
    static std::string formatSocketsMap(const std::map<std::string, unsigned int> &socketsMap) {
        std::stringstream result;
        for (auto &entry : socketsMap) {
            result << entry.second << " -> " << entry.first << ", ";
        }
        result << "";
        std::string partialResult = result.str();
        return partialResult.substr(0, partialResult.length() - 2);
    }
    static std::string formatParametersList(const std::vector<std::string> &parameterList) {
        std::stringstream result;
        for (const std::string &parameter : parameterList) {
            result << parameter << ", ";
        }
        std::string partialResult = result.str();
        return partialResult.substr(0, partialResult.length() - 2);
    }

    void MetaSection::print(raw_ostream &os) const {
        os << "META START";
        os << tabStr << "name: " << this->name;
        os << tabStr << "parameters: " << formatParametersList(this->globalParams);
        os << tabStr << "csockets: " << formatSocketsMap(this->classicalSocketsMap);
        os << tabStr << "epr_sockets: " << formatSocketsMap(this->eprsSocketsMap);
        os << "META END";
    }

    void HostSection::print(raw_ostream &os) const {
        // Iteratively print all the blocks
        for (const Block &block : this->hostBlocks) {
            os << block;
        }
    }

    void NetQASMSection::print(raw_ostream &os) const {
        // Iteratively print all the routines:
        for (const QuantumRoutine &routine : this->routines) {
            os << routine;
        }
    }
}