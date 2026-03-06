#ifndef QOALAHOST_HELPERS_H
#define QOALAHOST_HELPERS_H

#include <set>
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/Casting.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/AnalysisManager.h"
#include "mlir/Transforms/DialectConversion.h"

#include <scip/scip.h>

// TODO including "Dialect/QoalaHost/QoalaHost.h" creates
// circular inclusion issues at compile time. See qoala-kanban-board#110.
#define GET_OP_FWD_DEFINES
#include "Dialect/QoalaHost/QoalaHost.h.inc"

namespace qoala::analysis {
    namespace precedences {
        /**
         * Track precedences (data dependencies, predecessors, communication/entanglement ordering).
         * Make the precedences information available by inserting a "blk_meta"
         * operation At the beginning of each block.
         * @param moduleOp module to walk for tracking and adding precedences.
         */

        mlir::LogicalResult addPrecedences(mlir::ModuleOp &moduleOp, bool useOnlineScheduler = false);
    } // namespace precedences

    namespace qmemeff {
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
    } // namespace qmemeff

    /**
     * Build MILP-compatible block representations from the main function.
     * Used to analyze control/data dependencies and enable block reordering.
     */
    namespace reordering {
        enum class BlockType { CL, CC, QL, QC };

        enum class TaskGroup { C, Q };

        // Class to represent an operation for the MILP model
        class MILPOperation {
        public:
            MILPOperation(std::string id, const uint64_t duration):
                id_(std::move(id)), duration_(duration), op_(nullptr) { }

            [[nodiscard]]
            const std::string &getId() const {
                return id_;
            }

            [[nodiscard]]
            uint64_t getDuration() const {
                return duration_;
            }

            void setOperation(mlir::Operation *op) { op_ = op; }

            [[nodiscard]]
            mlir::Operation *getOperation() const {
                return op_;
            }

        private:
            std::string id_;
            uint64_t duration_;
            mlir::Operation *op_;
        };

        class MILPBlock; // forward declaration

        // Class to represent a Task for the MILP model
        class MILPTask {
        public:
            MILPTask(std::string id, MILPBlock *parent, const TaskGroup group):
                id_(std::move(id)), parent_block_(parent), group_(group) { }

            [[nodiscard]]
            const std::string &getId() const {
                return id_;
            }

            [[nodiscard]]
            TaskGroup getGroup() const {
                return group_;
            }

            void addOperation(MILPOperation *op) { operations_.push_back(op); }

            [[nodiscard]]
            const std::vector<MILPOperation *> &getOperations() const {
                return operations_;
            }

            [[nodiscard]]
            uint64_t getDuration() const {
                uint64_t dur = 0;
                for (const auto *op : operations_) {
                    dur += op->getDuration();
                }
                return dur;
            }

            [[nodiscard]]
            MILPBlock *getParentBlock() const {
                return parent_block_;
            }

        private:
            std::string id_;
            MILPBlock *parent_block_;
            TaskGroup group_;
            std::vector<MILPOperation *> operations_;
        };

        // Class to represent a block for the MILP model
        class MILPBlock {
        public:
            MILPBlock(std::string id, const BlockType type): id_(std::move(id)), type_(type), blk_(nullptr) { }

            [[nodiscard]]
            const std::string &getId() const {
                return id_;
            }

            [[nodiscard]]
            BlockType getType() const {
                return type_;
            }

            MILPOperation *addOperation(std::unique_ptr<MILPOperation> op) {
                MILPOperation *raw = op.get();
                operations_.push_back(std::move(op));
                return raw;
            }

            [[nodiscard]]
            const std::vector<std::unique_ptr<MILPOperation>> &getOperations() const {
                return operations_;
            }

            void addTask(std::unique_ptr<MILPTask> task) { tasks_.push_back(std::move(task)); }

            [[nodiscard]]
            const std::vector<std::unique_ptr<MILPTask>> &getTasks() const {
                return tasks_;
            }

            void setBlock(mlir::Block *block) { blk_ = block; }

            [[nodiscard]]
            mlir::Block *getBlock() const {
                return blk_;
            }

            [[nodiscard]]
            const MILPOperation *firstOp() const {
                return operations_.front().get();
            }

            [[nodiscard]]
            const MILPOperation *lastOp() const {
                return operations_.back().get();
            }

        private:
            std::string id_;
            BlockType type_;
            std::vector<std::unique_ptr<MILPOperation>> operations_;
            std::vector<std::unique_ptr<MILPTask>> tasks_;
            mlir::Block *blk_;
        };

        // Class to represent a qubit for the MILP model
        class MILPQubit {
        public:
            explicit MILPQubit(std::string id): id_(std::move(id)), alloc_op_(nullptr), meas_op_(nullptr) { }

            [[nodiscard]]
            const std::string &getId() const {
                return id_;
            }

            void setAllocation(MILPOperation *allocOp) { alloc_op_ = allocOp; }

            void setMeasurement(MILPOperation *measOp) { meas_op_ = measOp; }

            [[nodiscard]]
            MILPOperation *getAllocation() const {
                return alloc_op_;
            }

            [[nodiscard]]
            MILPOperation *getMeasurement() const {
                return meas_op_;
            }

            virtual ~MILPQubit() = default;

        private:
            std::string id_;
            MILPOperation *alloc_op_;
            MILPOperation *meas_op_;
        };

        // each pair (A, B) means "A must finish before B starts"
        using BlockPrecedence = std::pair<MILPBlock *, MILPBlock *>;
        using BlockPrecedenceList = std::vector<BlockPrecedence>;

        // Encapsulates the SCIP MILP modeling process
        class MILPModelBuilder {
        public:
            virtual ~MILPModelBuilder() = default;

            // Initialize SCIP and prepare internal state
            bool initialize();

            // Inject blocks, qubits and precedences
            void setProblemData(const std::vector<std::shared_ptr<MILPBlock>> &blocks,
                                const std::vector<std::shared_ptr<MILPQubit>> &qubits,
                                const BlockPrecedenceList &precedences) {
                blocks_ = blocks;
                qubits_ = qubits;
                precedences_ = precedences;
            };

            // Create all SCIP variables
            virtual void createVariables() = 0;

            // Add all model constraints
            virtual void addConstraints() = 0;

            // Configure SCIP objective = primary objective (lifetime minimization, etc.)
            virtual void setPrimaryObjective() = 0;

            // Optimize using SCIP
            bool optimize() const { return SCIPsolve(scip_) == SCIP_OKAY; }

            bool checkSolverStatus(mlir::ModuleOp *op) const;

            // Read back the exact primary objective value from the optimal solution.
            virtual double getPrimaryObjectiveValueFromSolution() const = 0;

            // Add linear constraint that "primary objective expression == zStar".
            // (To be called when we rebuild a *second* SCIP instance.)
            virtual void constrainPrimaryObjectiveTo(double zStar) = 0;

            // Wipe objective coeffs and install deterministic tiebreaker objective.
            virtual void setSecondaryObjectiveDeterministic() = 0;

            // Free SCIP variables and memory
            void cleanup();

            // Retrieve start time for a specific operation (by ID)
            double getOperationStartTime(const std::string &opId) const;

            std::vector<std::string> getOrderedBlocks() const;

        protected:
            std::vector<std::shared_ptr<MILPBlock>> blocks_;
            std::vector<std::shared_ptr<MILPQubit>> qubits_;
            BlockPrecedenceList precedences_;
            std::unordered_map<std::string, SCIP_VAR *> startVars_;
            SCIP *scip_ = nullptr;
        };

        class MILPBlockOrderModel : public MILPModelBuilder {
        public:
            MILPBlockOrderModel(): bigM_(0) { }

            ~MILPBlockOrderModel() override { cleanup(); }

            void createVariables() override;

            void addConstraints() override {
                // Call individual constraint-adding methods
                addIntraTaskOrderingConstraints();
                addBlockPrecedenceConstraints();
                addFCFSTaskConstraints();
                addIntraBlockSequencingConstraints();
            };

            void setPrimaryObjective() override;
            double getPrimaryObjectiveValueFromSolution() const override;
            void constrainPrimaryObjectiveTo(double zStar) override;
            void setSecondaryObjectiveDeterministic() override;

            // Provide the allocation order (from primary) to guide tie-breaking
            // allocOpIdsInOrder: list of Allocation op IDs sorted "earliest first" per primary solution
            void setPrimaryAllocationOrder(const std::vector<std::string> &allocOpIdsInOrder);

            std::vector<std::string> computeAllocationOrderFromSolution() const;

        private:
            // Specific constraints
            void addIntraTaskOrderingConstraints();

            void addBlockPrecedenceConstraints();

            void addFCFSTaskConstraints();

            void addIntraBlockSequencingConstraints();

            uint32_t bigM_;

            std::unordered_map<std::string, int> primaryAllocRank_;
        };

        class MILPBlockDeadlineModel : public MILPModelBuilder {
        public:
            MILPBlockDeadlineModel() = default;

            ~MILPBlockDeadlineModel() override = default;

            void createVariables() override;

            void addConstraints() override {
                addIntraTaskSequencingConstraints();
                addIntraBlockSequencingConstraints();
                addBlockPrecedenceConstraints();
                addFCFSConsistencyConstraints();
                addQubitLifetimeConstraints();
                addInterBlockGapConstraints();
                addProgramHorizonConstraint();
            };

            void setPrimaryObjective() override; // phase 1 objective
            double getPrimaryObjectiveValueFromSolution() const override;
            void constrainPrimaryObjectiveTo(double zStar) override;
            void setSecondaryObjectiveDeterministic() override;

            std::pair<std::unordered_map<std::string, uint32_t>, std::string> computeBlockDeadlines() const;

        private:
            std::unordered_map<std::string, SCIP_VAR *> gapVars_;
            SCIP_VAR *gminVar_ = nullptr;

            void addIntraTaskSequencingConstraints();

            void addIntraBlockSequencingConstraints();

            void addBlockPrecedenceConstraints();

            void addFCFSConsistencyConstraints();

            void addQubitLifetimeConstraints();
            void addInterBlockGapConstraints();
            void addProgramHorizonConstraint();

            double getProgramHorizon() const;
        };

        using Closure = std::set<std::pair<std::string, std::string>>;

        /**
         * Collects all NetQASM routines defined in the module and returns a map
         * from their symbol names to their corresponding operations.
         * This includes both `netqasm.local_routine` and `netqasm.request_routine`.
         * @param moduleOp The module from which to extract routines.
         * @returns A map from routine name (as string) to its defining operation.
         */
        llvm::StringMap<mlir::Operation *> collectRoutineMap(mlir::ModuleOp &moduleOp);

        /**
         * Constructs the MILP model from the given MLIR module. This includes building
         * MILP blocks, qubit usage, and block precedence constraints.
         * @param moduleOp The MLIR module to analyze.
         * @returns A tuple containing the constructed MILP blocks, qubits, precedence list,
         *          and a LogicalResult indicating success or failure.
         */
        std::tuple<std::vector<std::shared_ptr<MILPBlock>>, std::vector<std::shared_ptr<MILPQubit>>,
                   BlockPrecedenceList, llvm::StringMap<MILPBlock *>, mlir::LogicalResult>
        buildMILPFromMLIR(mlir::ModuleOp &moduleOp);

        /**
         * Constructs the MILP blocks from the given main function and the routine map.
         * @param mainFunc The main function.
         * @param routineMap a map from NetQASM routines' symbol names to their corresponding operations
         * @returns A tuple containing the constructed MILP blocks, a map between MLIR operations and MILP operations,
         * precedence list, unresolvedEdges, a map between MILP block ids and the blocks, and a LogicalResult indicating
         * success or failure.
         */
        std::tuple<std::vector<std::shared_ptr<MILPBlock>>, std::unordered_map<mlir::Operation *, MILPOperation *>,
                   BlockPrecedenceList, std::vector<std::pair<std::string, std::string>>, llvm::StringMap<MILPBlock *>,
                   mlir::LogicalResult>
        buildMilpBlocks(qoala::dialects::qoalahost::MainFuncOp &mainFunc,
                        const llvm::StringMap<mlir::Operation *> &routineMap);

        /**
         * Constructs a map between qubits as MLIR values and all the MLIR operations that have memory effects on them.
         * @param mainFunc The main function.
         * @param moduleOp The MLIR module to analyze.
         * @returns A map between the qubit MLIR values and a vector of MLIR operations that have memory effects on
         * them, and a LogicalResult indicating success or failure.
         */
        std::tuple<llvm::DenseMap<mlir::Value, std::vector<mlir::Operation *>>, mlir::LogicalResult>
        collectQubitUsage(qoala::dialects::qoalahost::MainFuncOp &mainFunc, mlir::ModuleOp &moduleOp);

        /**
         * Reorders the blocks in the given module based on the specified MILP solution order.
         * This mainly affects the block order inside MainFuncOp.
         * @param moduleOp The module whose blocks will be reordered.
         * @param orderedBlockIds A list of block IDs (as specified in BlkMeta) in the desired order.
         */
        mlir::LogicalResult reorderBlocksByMilpOrder(mlir::ModuleOp &moduleOp,
                                                     const std::vector<std::string> &orderedBlockIds);

        /**
         * Creates a precedence list (as block pairs) from a specified linear order of block IDs.
         * For each adjacent pair in the ordered list, creates a precedence edge (A -> B).
         * @param moduleOp The module to which the blocks belong to.
         * @param orderedBlockIds Vector of block IDs in desired execution order.
         * @param idToBlockMap Map from block IDs to MILPBlock pointers.
         * @returns A list of block precedence edges as (source, target) pairs.
         */
        BlockPrecedenceList createPrecedenceFromOrder(mlir::ModuleOp *moduleOp,
                                                      const std::vector<std::string> &orderedBlockIds,
                                                      const llvm::StringMap<MILPBlock *> &idToBlockMap);

        /**
         * Annotates each `qoalahost.blk_meta` operation in the module with deadline information.
         * Deadlines are relative to a reference block and stored in the `deadlines` attribute
         * as a dictionary mapping block ID to an integer value.
         * @param moduleOp The MLIR module containing blk_meta operations.
         * @param deadlines Map from block ID to computed deadline (as int).
         * @param refBlockId The block ID that serves as the reference (time 0).
         */
        void annotateBlockDeadlines(mlir::ModuleOp &moduleOp,
                                    const std::unordered_map<std::string, uint32_t> &deadlines,
                                    const std::string &refBlockId);

        /**
         * Moves all blocks containing entanglement request routines
         * (i.e., calls to `netqasm.request_routine`) to the beginning of
         * the `qoalahost.main_func`. This is done to ensure these blocks are
         * scheduled before others. The reordering is done before `blk_meta`
         * operations are inserted.
         * @param moduleOp The module whose main function will be reordered.
         */
        void groupEntanglementBlocksFirst(mlir::ModuleOp moduleOp);

        /**
         * Finds the qoalahost::MainFuncOp operation of the given module, and fixes
         * the position (moves it to the beginning, leaving it as the first) of the
         * block containing the qoalahost.remote_id_ref operations.
         * @param moduleOp The module on which to apply this fix.
         */
        void moveRemoteReferencesBlockToBegin(mlir::ModuleOp &moduleOp);
    } // namespace reordering

} // namespace qoala::analysis

#endif // QOALAHOST_HELPERS_H
