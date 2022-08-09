#include "mlir/IR/PatternMatch.h"

#include "Dialect/iqoala/iQoala.h"
#include "Dialect/iqoala/Transforms/iQoalaTransforms.h.inc"

using namespace mlir;
using namespace mlir::iqoala;

struct SimplifyRedundantTranspose : public mlir::OpRewritePattern<TransposeOp>
{
    /// We register this pattern to match every iqoala.transpose in the IR.
    /// The "benefit" is used by the framework to order the patterns and process
    /// them in order of profitability.
    SimplifyRedundantTranspose(mlir::MLIRContext *context)
        : OpRewritePattern<TransposeOp>(context, /*benefit=*/1) {}

    /// This method is attempting to match a pattern and rewrite it. The rewriter
    /// argument is the orchestrator of the sequence of rewrites. It is expected
    /// to interact with it to perform any changes to the IR from here.
    mlir::LogicalResult
    matchAndRewrite(TransposeOp op,
                    mlir::PatternRewriter &rewriter) const override
    {
        // Look through the input of the current transpose.
        mlir::Value transposeInput = op.getOperand();
        TransposeOp transposeInputOp = transposeInput.getDefiningOp<TransposeOp>();

        // Input defined by another transpose? If not, no match.
        if (!transposeInputOp)
            return failure();

        // Otherwise, we have a redundant transpose. Use the rewriter.
        mlir::Value input_op_operand = transposeInputOp.getOperand();

        op.emitRemark() << "defining op is also a transpose, replacing... \n"
                        << "original operand: " << input_op_operand;

        rewriter.replaceOp(op, {transposeInputOp.getOperand()});
        return success();
    }
};

void TransposeOp::getCanonicalizationPatterns(RewritePatternSet &results, MLIRContext *context)
{
    results.add<SimplifyRedundantTranspose>(context);
}

void ReshapeOp::getCanonicalizationPatterns(RewritePatternSet &results, MLIRContext *context)
{
    results.add<ReshapeReshapeOptPattern>(context);
}