#include <mlir/Transforms/GreedyPatternRewriteDriver.h>

#include "Analysis/QMem/Unfold.h"
#include "Dialect/Helpers/MIRToLIRHelperPasses.h"
#include "Dialect/QMem/QMem.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/DialectConversion.h"

#include "llvm/Support/Debug.h"

// Includes to be moved to the header when generalizing

#include "llvm/ADT/TypeSwitch.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"

using namespace mlir;
using namespace qoala::dialects::qmem;

#define DEBUG_TYPE "qmem-unfold-qmem-ops"

namespace qoala::analysis {
#define GEN_PASS_DEF_UNFOLDCLASSICALCOMMOPS
#include "Dialect/Helpers/HelperPasses.h.inc"

    using UnfoldSendIntsPattern = unfold::UnfoldSendPattern<SendIntsOp, SendIntOp, IntegerAttr>;
    using UnfoldSendFloatsPattern = unfold::UnfoldSendPattern<SendFloatsOp, SendFloatOp, FloatAttr>;

    static LogicalResult processUserOperation(Operation *userOp, DenseMap<Value, uint32_t> &oldRecvValues) {
        return llvm::TypeSwitch<Operation *, LogicalResult>(userOp)
                .Case([&](tensor::ExtractOp extractOp) {
                    // This is an extract; we are accessing one of the received values.
                    // We need to replace all the uses of the value yielded by this extract
                    // with the corresponding new "recv_int".

                    if (extractOp.getIndices().size() > 1) {
                        extractOp.emitOpError(
                                "Multi-dimension extraction of received valued is not supported.");
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

    class UnfoldSendPattern : public OpRewritePattern<RecvIntsOp> {
    public:
        using OpRewritePattern::OpRewritePattern;

        LogicalResult matchAndRewrite(RecvIntsOp sourceSendOp, PatternRewriter &rewriter) const override {
            // When unfolding a recv comm op, we need to track the usages of the operation
            DenseMap<uint32_t, Value> newRecvValues;
            DenseMap<Value, uint32_t> oldRecvValues;
            for (auto *userOp : sourceSendOp->getUsers()) {
                if (failed(processUserOperation(userOp, oldRecvValues))) {
                    sourceSendOp.emitOpError("Cannot unfold operation.");
                    return failure();
                }
            }
            // Create the new recv_int operations
            for (uint32_t i = 0; i < sourceSendOp.getLengthAttr().getInt(); i++) {
                auto newRecvInt = rewriter.create<RecvIntOp>(sourceSendOp.getLoc(), sourceSendOp.getRemoteAttr());
                newRecvValues.try_emplace(i, newRecvInt.getResult());
            }

            // Replace the old extract operations with the value from the new recv_int
            for (auto [oldValue, index] : oldRecvValues) {
                rewriter.replaceAllUsesWith(oldValue, newRecvValues[index]);
                rewriter.eraseOp(oldValue.getDefiningOp());
            }
            rewriter.eraseOp(sourceSendOp.getOperation());
            return success();
        }
    };

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
        patterns.add<UnfoldSendPattern>(&context);

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
