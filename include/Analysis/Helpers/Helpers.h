#ifndef QOALA_ANALYSIS_MLIR_HELPERS_H
#define QOALA_ANALYSIS_MLIR_HELPERS_H

#include "mlir/IR/Operation.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/Dialect/Affine/IR/AffineOps.h"

using namespace mlir;

namespace qoala::helpers {
    bool operationIsNotFromCommonDialects(Operation &);
    std::string getAllowedDialectNames();
}


#endif //QOALA_ANALYSIS_MLIR_HELPERS_H
