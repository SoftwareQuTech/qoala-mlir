#ifndef HELPERS_H
#define HELPERS_H

#include "llvm/Support/Casting.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/IR/BuiltinOps.h"
#include <scip/scip.h>
#include <scip/scipdefplugins.h>

namespace qoala::analysis {
    namespace isolate {
        mlir::Operation *getNextOperation(mlir::Operation *op);
        /**
         * Isolate the given operation in its own block.
         * @param opToIsolate Operation to isolate in its own block.
         * @param rewriter The convertion patter rewriter to create new block
         * terminators as needed.
         */
        void isolateOp(mlir::Operation *opToIsolate, mlir::ConversionPatternRewriter &rewriter);

        using OpCheck = bool (*)(mlir::Operation *);

        /**
         * Isolate all the nested operations of a given type. The search will start from
         * a given base operation and isolate all the found operations into a single
         * block. Since creating new isolated blocks might create new blocks with no
         * terminator operations, this method will also create new terminator operations
         * as needed. This function also allows passing an extra check function to
         * exclude the isolation of certain nested operations despite they match the
         * types.
         * @tparam FuncOpTy Type of the base function to start exploring for nested
         * operations. This type should usually be "qoalahost::MainFuncOp", but can
         * easily be other type with a nested region.
         * @tparam OpTy Types of the operations to isolate.
         * @param baseOp The base operation to explore for functions to isolate.
         * @param rewriter A ConversionPatternRewriter object to create new block
         * terminators as needed
         * @param extraCheck An optional extra check function. If this given check
         * returns false, the involved operation will not be marked to isolate, even if
         * it is of the good type.
         */
        template<typename FuncOpTy, typename... OpTy>
        void isolateOpsInNewBlocks(FuncOpTy &baseOp, mlir::ConversionPatternRewriter &rewriter,
                                   const std::optional<OpCheck> extraCheck = std::nullopt) {
            auto mainFunction = llvm::dyn_cast<FuncOpTy>(&baseOp);
            assert(mainFunction && "Trying to isolate the operations on an operation "
                                   "that is not a qoalahost.main_func");
            mlir::SmallVector<mlir::Operation *> opsToIsolate;

            // To correctly isolate the op, we will simply "mark them", since modifying
            // the structure of the blocks will invalidate the internal iterators
            // of the walk functions.
            mainFunction->walk([&](mlir::Operation *op) {
                if (llvm::isa<OpTy...>(op)) {
                    if (extraCheck.has_value() && !extraCheck.value()(op)) {
                        return;
                    }
                    opsToIsolate.push_back(op);
                }
            });

            // Once we marked all the ops, we isolate them
            for (mlir::Operation *opToIsolate: opsToIsolate) {
                isolateOp(opToIsolate, rewriter);
            }
        }
    } // namespace isolate

    namespace precedences {
        /**
         * Track precedences (data dependencies, predecessors, communication/entanglement ordering).
         * Make the precedences information available by inserting a "blk_meta"
         * operation At the beginning of each block.
         * @param moduleOp module to walk for tracking and adding precedences.
         */
        mlir::LogicalResult addPrecedences(mlir::ModuleOp &moduleOp);
    } // namespace precedences

    /**
     * Build MILP-compatible block representations from the main function.
     * Used to analyze control/data dependencies and enable block reordering.
     */
    namespace reordering {
        // Forward declarations
        class MILPOperation;
        class MILPBlock;

        enum class OpType { CL, CC, QL, QC, UNKNOWN };

        // Represents a single operation
        class MILPOperation {
        public:
            MILPOperation(const std::string &id, OpType type, double duration);

            const std::string &getId() const;
            OpType getType() const;
            double getDuration() const;

            void setStartTime(double startTime);
            double getStartTime() const;

        private:
            std::string id_;
            OpType type_;
            double duration_;
            double start_time_; // decision variable: start(o)
        };

        // Represents a group of operations in a block with same type (CL or Q)
        class MILPTask {
        public:
            MILPTask(int id, MILPBlock *parent, const std::string &group); // group: "C" or "Q"

            int getId() const;
            const std::string &getGroup() const;

            void addOperation(MILPOperation *op);
            const std::vector<MILPOperation *> &getOperations() const;

            MILPOperation *getFirstOperation() const;
            double getDuration() const;

            MILPBlock *getParentBlock() const;

        private:
            int id_;
            std::string group_;
            std::vector<MILPOperation *> operations_;
            MILPBlock *parent_block_;
        };

        // Represents a block, an ordered sequence of operations
        class MILPBlock {
        public:
            MILPBlock(const std::string &id, OpType type);

            const std::string &getId() const;
            OpType getType() const;

            void addOperation(MILPOperation *op);
            const std::vector<MILPOperation *> &getOperations() const;

            void addTask(MILPTask *task);
            const std::vector<MILPTask *> &getTasks() const;

        private:
            std::string id_;
            OpType type_;
            std::vector<MILPOperation *> operations_;
            std::vector<MILPTask *> tasks_;
        };

        // Represents a logical qubit with allocation and measurement operations
        class MILPQubit {
        public:
            MILPQubit(const std::string &id);

            const std::string &getId() const;

            void setAllocation(MILPOperation *allocOp);
            void setMeasurement(MILPOperation *measOp);

            MILPOperation *getAllocation() const;
            MILPOperation *getMeasurement() const;

            double computeLifetime() const; // start(meas) + dur(meas) - (start(alloc) + dur(alloc))

        private:
            std::string id_;
            MILPOperation *alloc_op_; // from O_Q
            MILPOperation *meas_op_; // from O_QL
        };

        std::tuple<std::vector<std::shared_ptr<MILPBlock>>, std::vector<std::shared_ptr<MILPQubit>>,
                   mlir::LogicalResult>
        buildMILPFromMLIR(mlir::ModuleOp module);

        // Encapsulates the SCIP MILP modeling process
        class MILPModelBuilder {
        public:
            MILPModelBuilder();
            ~MILPModelBuilder();

            // Initialize SCIP and setup base model
            bool initialize();

            // Inject blocks and qubits to be used in modeling
            void setProblemData(const std::vector<std::shared_ptr<MILPBlock>> &blocks,
                                const std::vector<std::shared_ptr<MILPQubit>> &qubits);

            // Create SCIP variables for each operation
            void createVariables();

            // Add all constraints to the model
            void addConstraints();

            // Add a single group of constraints (optional modular form)
            void addIntraTaskOrderingConstraints();
            void addBlockPrecedenceConstraints();
            void addFCFSTaskConstraints();
            void addIntraBlockSequencingConstraints();

            // Define the objective function (e.g., minimize total qubit lifetime)
            void setObjective();

            // Run the solver
            bool optimize();

            // Retrieve start time for a specific operation (by ID)
            double getOperationStartTime(const std::string &opId) const;

            // Cleanup and destroy SCIP
            void cleanup();

        private:
            SCIP *scip_; // main SCIP context
            std::vector<std::shared_ptr<MILPBlock>> blocks_;
            std::vector<std::shared_ptr<MILPQubit>> qubits_;

            // Map from operation ID to SCIP variable
            std::unordered_map<std::string, SCIP_VAR *> startVars_;

            // Utility to create SCIP variable
            SCIP_VAR *createContinuousVariable(const std::string &name, double lb, double ub);
        };
    } // namespace reordering
} // namespace qoala::analysis

#endif // HELPERS_H
