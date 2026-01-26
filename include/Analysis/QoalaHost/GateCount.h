#ifndef QOALA_MLIR_GATECOUNT_H
#define QOALA_MLIR_GATECOUNT_H

#include "mlir/IR/BuiltinOps.h"

namespace qoala::analysis::gatecount {
    class QoalaHostGateCount {
    public:
        explicit QoalaHostGateCount(mlir::Operation *op);

        [[nodiscard]]
        const llvm::StringMap<uint32_t> &getDetailedOneQubitGateCount() const {
            return detailedOneQubitGateCount;
        }

        [[nodiscard]]
        const llvm::StringMap<uint32_t> &getDetailedTwoQubitGateCount() const {
            return detailedTwoQubitGateCount;
        }

        [[nodiscard]]
        const llvm::StringMap<uint32_t> &getDetailedGateCount() const {
            return detailedGateCount;
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
        llvm::StringMap<uint32_t> detailedGateCount;
        llvm::StringMap<uint32_t> detailedOneQubitGateCount;
        llvm::StringMap<uint32_t> detailedTwoQubitGateCount;
    };
} // namespace qoala::analysis::gatecount

#endif // QOALA_MLIR_GATECOUNT_H
