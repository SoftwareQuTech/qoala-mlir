#ifndef QNET_HELPERS_H
#define QNET_HELPERS_H

#include "llvm/ADT/StringMap.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/BuiltinOps.h"

namespace qoala::analysis::qnet::gatecount {
    // Forward declarations for internal helper functions
    // These are defined in GateCount.cpp and have mutual recursion:
    // visitOperationsInRegion calls visitScfIfOp for scf.if operations
    // visitScfIfOp calls visitOperationsInRegion for branch regions
    void visitOperationsInRegion(mlir::Region &region, llvm::DenseMap<mlir::Value, uint32_t> &opResToId, uint32_t &qId,
                                 uint32_t &gateCount, uint32_t &oneQubitGateCount, uint32_t &twoQubitGateCount,
                                 llvm::DenseMap<uint32_t, uint32_t> &detailedGateCount,
                                 llvm::DenseMap<uint32_t, uint32_t> &detailedOneQubitGateCount,
                                 llvm::DenseMap<uint32_t, uint32_t> &detailedTwoQubitGateCount);

    void visitScfIfOp(mlir::scf::IfOp ifOp, llvm::DenseMap<mlir::Value, uint32_t> &opResToId, uint32_t &gateCount,
                      uint32_t &oneQubitGateCount, uint32_t &twoQubitGateCount,
                      llvm::DenseMap<uint32_t, uint32_t> &detailedGateCount,
                      llvm::DenseMap<uint32_t, uint32_t> &detailedOneQubitGateCount,
                      llvm::DenseMap<uint32_t, uint32_t> &detailedTwoQubitGateCount);

    class QNetGateCount {
    public:
        explicit QNetGateCount(mlir::Operation *op);

        [[nodiscard]]
        const llvm::DenseMap<uint32_t, uint32_t> &getDetailedOneQubitGateCount() const {
            return detailedOneQubitGateCount;
        }

        [[nodiscard]]
        const llvm::DenseMap<uint32_t, uint32_t> &getDetailedTwoQubitGateCount() const {
            return detailedTwoQubitGateCount;
        }

        [[nodiscard]]
        const llvm::DenseMap<uint32_t, uint32_t> &getDetailedGateCount() const {
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
        llvm::DenseMap<uint32_t, uint32_t> detailedGateCount;
        llvm::DenseMap<uint32_t, uint32_t> detailedOneQubitGateCount;
        llvm::DenseMap<uint32_t, uint32_t> detailedTwoQubitGateCount;
    };
} // namespace qoala::analysis::qnet::gatecount

#endif // QNET_HELPERS_H
