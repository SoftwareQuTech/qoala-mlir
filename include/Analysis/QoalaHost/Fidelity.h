#ifndef QOALA_MLIR_FIDELITY_H
#define QOALA_MLIR_FIDELITY_H

#include "mlir/Pass/AnalysisManager.h"

namespace qoala::analysis::fidelity {
    class QoalaHostEstimateSuccProb {
    public:
        explicit QoalaHostEstimateSuccProb(mlir::Operation *op, mlir::AnalysisManager &am);

        [[nodiscard]]
        bool isInvalidated(const mlir::AnalysisManager::PreservedAnalyses &pa) const;

        [[nodiscard]]
        float getESP() const {
            return totalEsp;
        };

    private:
        float totalEsp = 1.0;
    };
} // namespace qoala::analysis::fidelity

#endif // QOALA_MLIR_FIDELITY_H
