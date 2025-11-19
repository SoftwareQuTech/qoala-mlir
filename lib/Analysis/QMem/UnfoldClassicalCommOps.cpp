#include "Analysis/QMem/Unfold.h"
#include "Dialect/Helpers/MIRToLIRHelperPasses.h"
#include "Dialect/QMem/QMem.h"
#include "llvm/ADT/TypeSwitch.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "qmem-unfold-qmem-ops"

using namespace mlir;
using namespace qoala::dialects::qmem;

namespace qoala::analysis {
#define GEN_PASS_DEF_UNFOLDCLASSICALCOMMOPS
#include "Dialect/Helpers/HelperPasses.h.inc"

    // This function might wrongly be marked as unused. It is used by the templates instantiated below
    static LogicalResult processUserOperation(Operation *userOp, DenseMap<Value, uint32_t> &oldRecvValues) {
        return llvm::TypeSwitch<Operation *, LogicalResult>(userOp)
                .Case([&](tensor::ExtractOp extractOp) {
                    // This is an extract; we are accessing one of the received values.
                    // We need to replace all the uses of the value yielded by this extract
                    // with the corresponding new "recv_int".

                    if (extractOp.getIndices().size() > 1) {
                        extractOp.emitOpError("Multi-dimension extraction of received valued is not supported.");
                        return failure();
                    }

                    Operation *indexDefiningOp = extractOp.getIndices()[0].getDefiningOp();
                    if (auto indexOp = dyn_cast<arith::ConstantOp>(indexDefiningOp)) {
                        TypedAttr valueAttr = indexOp.getValue();
                        if (const auto indexAttr = dyn_cast<IntegerAttr>(valueAttr)) {
                            oldRecvValues.try_emplace(extractOp.getResult(), static_cast<uint32_t>(indexAttr.getInt()));
                            return success();
                        }
                        // valueAttr is not an IntegerAttr
                        indexOp.emitOpError("Index-defining operation does not define an integer.");
                        return failure();
                    }
                    // indexOp is not an arith.constant op
                    indexDefiningOp->emitOpError("Recv index is not known at compile time.");
                    return failure();
                })
                .Default([](Operation *operation) {
                    operation->emitOpError("Unknown way to translate a QRemote operation to iQoala.");
                    return failure();
                });
    }

    // Instantiate the templates for unfolding send/recv_ints/floats
    using UnfoldSendIntsPattern = unfold::UnfoldSendPattern<SendIntsOp, SendIntOp, IntegerAttr>;
    using UnfoldSendFloatsPattern = unfold::UnfoldSendPattern<SendFloatsOp, SendFloatOp, FloatAttr>;
    using UnfoldRecvIntsPattern = unfold::UnfoldRecvPattern<RecvIntsOp, RecvIntOp>;
    using UnfoldRecvFloatsPattern = unfold::UnfoldRecvPattern<RecvFloatsOp, RecvFloatOp>;

    class UnfoldClassicalCommOpsPass : public impl::UnfoldClassicalCommOpsBase<UnfoldClassicalCommOpsPass> {
    public:
        using UnfoldClassicalCommOpsBase::UnfoldClassicalCommOpsBase;
        void runOnOperation() override;
    };

    void UnfoldClassicalCommOpsPass::runOnOperation() {
        const FuncOp funcOp = this->getOperation();
        MLIRContext &context = this->getContext();

        RewritePatternSet patterns(&context);
        patterns.add<UnfoldSendIntsPattern, UnfoldSendFloatsPattern>(&context);
        patterns.add<UnfoldRecvIntsPattern, UnfoldRecvFloatsPattern>(&context);

        GreedyRewriteConfig cfg;
        // If we don't disable region simplification, the folding of the code will be more aggressive
        // simplifying entire blocks if possible:
        // E.g, in test/Conversion/MIRtoLIR/BlkMeta/MIRtoLIR-precedences-predecessors.mlir
        // the aggressive folding will merge both "then" and "else" blocks into a single one,
        // inserting a block argument. That makes the test fail, since it expects 4 blocks, not 3.
        cfg.enableRegionSimplification = false;

        if (failed(applyPatternsAndFoldGreedily(funcOp, std::move(patterns), cfg))) {
            signalPassFailure();
        }
    }
} // namespace qoala::analysis
