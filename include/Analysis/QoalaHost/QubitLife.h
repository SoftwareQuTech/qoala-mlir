#ifndef QOALA_MLIR_QUBITLIFE_H
#define QOALA_MLIR_QUBITLIFE_H

#include "mlir/IR/BuiltinOps.h"

#include "Analysis/QoalaHost/Helpers.h"
    
namespace qoala::analysis::qubitlife {
    // Conceptually different from reordering::MILPTask
    // Represent smaller tasks that alwasy end with either
    // an init, measure or two-quit op.
    class Task {
    public:
        explicit Task(std::string n, const uint32_t t): name(std::move(n)), time(t) { }

        [[nodiscard]]
        std::string getName() const {
            return this->name;
        }

        [[nodiscard]]
        uint32_t getTime() const {
            return this->time;
        }

    private:
        std::string name;
        uint32_t time;
    };

    class LiveQubit : public reordering::MILPQubit {
        /**
         * Class used to keep track of the last two-qubit op applied to a qubit.
         */

    public:
        explicit LiveQubit(const std::string &id): reordering::MILPQubit(id), twoQubitOp_(nullptr) { }

        void setTwoQubitOp(reordering::MILPOperation *twoQubitOp) { this->twoQubitOp_ = twoQubitOp; }

        [[nodiscard]]
        reordering::MILPOperation *getTwoQubitOp() const {
            return this->twoQubitOp_;
        }

    private:
        reordering::MILPOperation *twoQubitOp_;
    };

    class QoalaHostQubitLifetime {
    public:
        explicit QoalaHostQubitLifetime(mlir::Operation *op);

        /**
         * Return the computed qubit life times.
         * @return A map with the computed qubit life times
         */
        [[nodiscard]]
        std::unordered_map<std::string, uint32_t> getLifetimes() const {
            return this->qubitLifetimes;
        }

    private:
        // A map from qubit IDs to their lifetimes.
        std::unordered_map<std::string, uint32_t> qubitLifetimes;
    };

} // namespace qoala::analysis::qubitlife

#endif // QOALA_MLIR_QUBITLIFE_H
