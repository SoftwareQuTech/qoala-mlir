#ifndef QNET_HELPERS_H
#define QNET_HELPERS_H

#include "llvm/ADT/StringMap.h"
#include "mlir/IR/BuiltinOps.h"

namespace qoala::analysis {
    namespace gatecount {
        class QNetGateCount {
        public:
            explicit QNetGateCount(mlir::Operation *op);

            [[nodiscard]]
            const llvm::StringMap<uint32_t> &getOneQubitGateCounts() const {
                return qubitToOneGateCount;
            }

            [[nodiscard]]
            const llvm::StringMap<uint32_t> &getTwoQubitGateCounts() const {
                return qubitToTwoGateCount;
            }

            [[nodiscard]]
            const llvm::StringMap<uint32_t> &getGateCounts() const {
                return qubitToGateCount;
            };

            [[nodiscard]]
            uint32_t getGateCount() const {
                return gateCount;
            };

            [[nodiscard]]
            uint32_t getOneQubitGateCount() const {
                return oneQubitGateCount;
            };

            [[nodiscard]]
            uint32_t getTwoQubitGateCount() const {
                return twoQubitGateCount;
            };

        private:
            uint32_t gateCount = 0;
            uint32_t oneQubitGateCount = 0;
            uint32_t twoQubitGateCount = 0;
            llvm::StringMap<uint32_t> qubitToGateCount;
            llvm::StringMap<uint32_t> qubitToOneGateCount;
            llvm::StringMap<uint32_t> qubitToTwoGateCount;
        };
    } // namespace gatecount
} // namespace qoala::analysis

#endif // QNET_HELPERS_H
