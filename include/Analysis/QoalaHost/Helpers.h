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
        enum class OpType { CL, CC, QL, QC };
        enum class TaskGroup { C, Q };

        // === MILPOperation ===
        class MILPOperation {
        public:
            MILPOperation(const std::string &id, OpType type, double duration) :
                id_(id), type_(type), duration_(duration), start_time_(0.0), op_(nullptr) {}

            const std::string &getId() const { return id_; }
            OpType getType() const { return type_; }
            double getDuration() const { return duration_; }

            void setStartTime(double startTime) { start_time_ = startTime; }
            double getStartTime() const { return start_time_; }

            void setOperation(mlir::Operation *op) { op_ = op; }
            mlir::Operation *getOperation() const { return op_; }

        private:
            std::string id_;
            OpType type_;
            double duration_;
            double start_time_;
            mlir::Operation *op_;
        };

        // === MILPTask ===
        class MILPBlock; // forward declaration

        class MILPTask {
        public:
            MILPTask(std::string id, MILPBlock *parent, const TaskGroup group) :
                id_(id), parent_block_(parent), group_(group) {}

            const std::string &getId() const { return id_; }
            TaskGroup getGroup() const { return group_; }

            void addOperation(MILPOperation *op) { operations_.push_back(op); }
            const std::vector<MILPOperation *> &getOperations() const { return operations_; }

            MILPOperation *getFirstOperation() const { return operations_.empty() ? nullptr : operations_.front(); }

            double getDuration() const {
                double dur = 0.0;
                for (auto *op: operations_)
                    dur += op->getDuration();
                return dur;
            }

            MILPBlock *getParentBlock() const { return parent_block_; }

        private:
            std::string id_;
            TaskGroup group_;
            std::vector<MILPOperation *> operations_;
            MILPBlock *parent_block_;
        };

        // === MILPBlock ===
        class MILPBlock {
        public:
            MILPBlock(const std::string &id, OpType type) : id_(id), type_(type), blk_(nullptr) {}

            const std::string &getId() const { return id_; }
            OpType getType() const { return type_; }

            void addOperation(MILPOperation *op) { operations_.push_back(op); }
            const std::vector<MILPOperation *> &getOperations() const { return operations_; }

            void addTask(std::unique_ptr<MILPTask> task) {
                tasks_.push_back(std::move(task));
            }
            const std::vector<std::unique_ptr<MILPTask>> &getTasks() const { return tasks_; }

            void setBlock(mlir::Block *block) { blk_ = block; }
            mlir::Block *getBlock() const { return blk_; }

        private:
            std::string id_;
            OpType type_;
            std::vector<MILPOperation *> operations_;
            std::vector<std::unique_ptr<MILPTask>> tasks_;
            mlir::Block *blk_;
        };

        // === MILPQubit ===
        class MILPQubit {
        public:
            MILPQubit(const std::string &id) : id_(id), alloc_op_(nullptr), meas_op_(nullptr) {}

            const std::string &getId() const { return id_; }

            void setAllocation(MILPOperation *allocOp) { alloc_op_ = allocOp; }
            void setMeasurement(MILPOperation *measOp) { meas_op_ = measOp; }

            MILPOperation *getAllocation() const { return alloc_op_; }
            MILPOperation *getMeasurement() const { return meas_op_; }

            double computeLifetime() const {
                if (!alloc_op_ || !meas_op_)
                    return 0.0;
                return (meas_op_->getStartTime() + meas_op_->getDuration()) -
                       (alloc_op_->getStartTime() + alloc_op_->getDuration());
            }

        private:
            std::string id_;
            MILPOperation *alloc_op_;
            MILPOperation *meas_op_;
        };

        // each pair (A, B) means "A must finish before B starts"
        using BlockPrecedence = std::pair<MILPBlock *, MILPBlock *>;
        using BlockPrecedenceList = std::vector<BlockPrecedence>;

        std::tuple<std::vector<std::shared_ptr<MILPBlock>>, std::vector<std::shared_ptr<MILPQubit>>,
                   BlockPrecedenceList, mlir::LogicalResult>
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
