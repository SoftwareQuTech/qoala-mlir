#ifndef QOALA_MLIR_EXPANSION_H
#define QOALA_MLIR_EXPANSION_H

#include "llvm/ADT/TypeSwitch.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/PatternMatch.h"

namespace qoala::analysis {
    // Declaration that needs to be implemented in the file including this header
    // We avoid placing that code in this header to avoid ending with multiple definitions of
    // the same function in different files.
    // That's also why we declare this function as static, since the symbol only makes sense
    // in the translation unit where the this function is defined.
    static mlir::LogicalResult processUserOperation(mlir::Operation *userOp,
                                                    mlir::DenseMap<mlir::Value, uint32_t> &oldRecvValues);

    namespace unfold {
        template<typename SourceType, typename TargetType>
        class UnfoldRecvPattern : public mlir::OpRewritePattern<SourceType> {
        public:
            using mlir::OpRewritePattern<SourceType>::OpRewritePattern;

            mlir::LogicalResult matchAndRewrite(SourceType sourceSendOp,
                                                mlir::PatternRewriter &rewriter) const override {
                // When unfolding a recv comm op, we need to track the usages of the operation
                mlir::DenseMap<uint32_t, mlir::Value> newRecvValues;
                mlir::DenseMap<mlir::Value, uint32_t> oldRecvValues;
                for (auto *userOp : sourceSendOp->getUsers()) {
                    if (failed(analysis::processUserOperation(userOp, oldRecvValues))) {
                        sourceSendOp.emitOpError("Cannot unfold operation.");
                        return mlir::failure();
                    }
                }
                // Create the new recv_int operations
                for (uint32_t i = 0; i < sourceSendOp.getLengthAttr().getInt(); i++) {
                    auto newRecvInt = rewriter.create<TargetType>(sourceSendOp.getLoc(), sourceSendOp.getRemoteAttr());
                    newRecvValues.try_emplace(i, newRecvInt.getResult());
                }

                // Replace the old extract operations with the value from the new recv_int
                for (auto [oldValue, index] : oldRecvValues) {
                    rewriter.replaceAllUsesWith(oldValue, newRecvValues[index]);
                    rewriter.eraseOp(oldValue.getDefiningOp());
                }
                rewriter.eraseOp(sourceSendOp.getOperation());
                return mlir::success();
            }
        };

        template<typename SourceType, typename TargetType, typename BaseAttrType>
        class UnfoldSendPattern : public mlir::OpRewritePattern<SourceType> {
        public:
            using mlir::OpRewritePattern<SourceType>::OpRewritePattern;

            mlir::LogicalResult matchAndRewrite(SourceType sourceSendOp,
                                                mlir::PatternRewriter &rewriter) const override {
                std::vector<mlir::Operation *> opsToDelete;
                // For a multi-value send comm op, we expect an "assembler" op, which packs the values
                // in a tensor/vector.
                llvm::TypeSwitch<mlir::Operation *>(sourceSendOp.getCin().getDefiningOp())
                        .Case([&](mlir::arith::ConstantOp constantOp) {
                            // If the values are all constants, then we expect the tensor/vector is
                            // assembled using an arith.constant dense operation
                            if (const auto denseElements = mlir::cast<mlir::DenseElementsAttr>(constantOp.getValue())) {
                                for (auto val : denseElements.getValues<BaseAttrType>()) {
                                    auto newIntVal = rewriter.create<mlir::arith::ConstantOp>(constantOp.getLoc(), val);
                                    rewriter.create<TargetType>(constantOp.getLoc(), newIntVal,
                                                                sourceSendOp.getRemote());
                                }
                                if (constantOp->hasOneUse()) {
                                    // If the constantOp has one use, it *must* be sendIntsOp. If so, delete it
                                    assert(*constantOp->getUsers().begin() == sourceSendOp &&
                                           "[QMem - Unfold comm ops] The single usage of arith.constant is not the op "
                                           "being deleted.");
                                    opsToDelete.push_back(constantOp);
                                }
                            } else {
                                assert(false && "[QMem - Unfold comm ops] arith.constant does not use a dense "
                                                "attribute argument.");
                            }
                        })
                        // TODO - Figure out how runtime values are packed into a Tensor/Vector
                        .Default([](mlir::Operation *operation) {
                            operation->emitOpError("Unknown way to translate a QRemote operation to iQoala");
                        });
                // First, delete the leaf operation
                rewriter.eraseOp(sourceSendOp);
                // Then any operations that yield a value used exclusively by the leaf operation
                // TODO - Do this recursively if necessary
                for (const auto opToDelete : opsToDelete) {
                    rewriter.eraseOp(opToDelete);
                }
                return mlir::success();
            }
        };
    } // namespace unfold
} // namespace qoala::analysis

#endif // QOALA_MLIR_EXPANSION_H
