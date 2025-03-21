#ifndef HELPERS_H
#define HELPERS_H

#include "mlir/Transforms/DialectConversion.h"
#include "mlir/IR/BuiltinOps.h"

namespace qoala::analysis::isolate {
    void isolateOps(mlir::Operation *funcOp, mlir::ConversionPatternRewriter &rewriter);
}

#endif //HELPERS_H
