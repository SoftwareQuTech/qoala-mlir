#ifndef QOALA_MLIR_EXPANSION_H
#define QOALA_MLIR_EXPANSION_H

#include "llvm/ADT/TypeSwitch.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/PatternMatch.h"

using namespace mlir;

namespace qoala::analysis::unfold {
    template<typename SourceType, typename TargetType, typename BaseAttrType>
    class UnfoldSendPattern : public OpRewritePattern<SourceType> {
    public:
        using OpRewritePattern<SourceType>::OpRewritePattern;

        LogicalResult matchAndRewrite(SourceType sourceSendOp, PatternRewriter &rewriter) const override {
            std::vector<Operation *> opsToDelete;
            // For a multi-value send comm op, we expect an "assembler" op, which packs the values
            // in a tensor/vector.
            llvm::TypeSwitch<Operation *>(sourceSendOp.getCin().getDefiningOp())
                    .Case([&](arith::ConstantOp constantOp) {
                        // If the values are all constants, then we expect the tensor/vector is
                        // assembled using an arith.constant dense operation
                        if (const auto denseElements = dyn_cast<DenseElementsAttr>(constantOp.getValue())) {
                            for (auto val : denseElements.getValues<BaseAttrType>()) {
                                auto newIntVal = rewriter.create<arith::ConstantOp>(constantOp.getLoc(), val);
                                rewriter.create<TargetType>(constantOp.getLoc(), newIntVal, sourceSendOp.getRemote());
                            }
                            if (constantOp->hasOneUse()) {
                                // If the constantOp has one use, it *must* be sendIntsOp. If so, delete it
                                assert(*constantOp->getUsers().begin() == sourceSendOp &&
                                       "[QMem - Unfold comm ops] The single usage of arith.constant is not the op "
                                       "being deleted.");
                                opsToDelete.push_back(constantOp);
                            }
                        } else {
                            assert(false &&
                                   "[QMem - Unfold comm ops] arith.constant does not use a dense attribute argument.");
                        }
                    })
                    // TODO - Figure out how runtime values are packed into a Tensor/Vector
                    .Default([](Operation *operation) {
                        operation->emitOpError("Unknown way to translate a QRemote operation to iQoala: '")
                                << *operation << "'\n";
                    });
            // First, delete the leaf operation
            rewriter.eraseOp(sourceSendOp);
            // Then any operations that yield a value used exclusively by the leaf operation
            // TODO - Do this recursively if necessary
            for (const auto opToDelete : opsToDelete) {
                rewriter.eraseOp(opToDelete);
            }
            return success();
        }
    };
} // namespace qoala::analysis::unfold

#endif // QOALA_MLIR_EXPANSION_H
