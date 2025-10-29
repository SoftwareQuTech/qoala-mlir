#ifndef QNET_HELPERS_H
#define QNET_HELPERS_H

#include "mlir/IR/BuiltinOps.h"

namespace qoala::analysis {
    namespace gatecount {
        class QNetGateCount {
        public:
            explicit QNetGateCount(mlir::Operation *op);

            [[nodiscard]]
            uint32_t getOneQubitGateCount() const {
                // return virtualQubits;
                return 0;
            }

            [[nodiscard]]
            uint32_t getTwoQubitGateCount() const {
                // return physicalQubits;
                return 0;
            }

            [[nodiscard]]
            float getGateCount() const {
                return 0;
            };
        private:
            int gateCount;
            std::unordered_map<std::string, int> qubitToGateCount;
            std::unordered_map<std::string, int> qubitToOneGateCount;
            std::unordered_map<std::string, int> qubitToTwoGateCount;
        };
    } // namespace gatecount
} // namespace qoala::analysis

#endif // QNET_HELPERS_H