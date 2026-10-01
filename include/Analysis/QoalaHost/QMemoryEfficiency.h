#ifndef QOALA_MLIR_QMEMORYEFFICIENCY_H
#define QOALA_MLIR_QMEMORYEFFICIENCY_H

#include <mlir/IR/BuiltinOps.h>

namespace qoala::analysis::qmemeff {
    class QoalaHostQMemoryEfficiency {
    public:
        explicit QoalaHostQMemoryEfficiency(mlir::Operation *op);

        [[nodiscard]]
        uint32_t getVirtualQubitCount() const {
            return virtualQubits;
        }

        [[nodiscard]]
        uint32_t getPhysicalQubitCount() const {
            return physicalQubits;
        }

        [[nodiscard]]
        float getEfficiency() const;

    private:
        uint32_t virtualQubits = 0;
        uint32_t physicalQubits = 0;
    };
} // namespace qoala::analysis::qmemeff

#endif // QOALA_MLIR_QMEMORYEFFICIENCY_H
