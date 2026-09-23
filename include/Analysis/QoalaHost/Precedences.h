#ifndef QOALA_MLIR_PRECEDENCES_H
#define QOALA_MLIR_PRECEDENCES_H

#include "mlir/IR/BuiltinOps.h"
#include "mlir/Support/LogicalResult.h"

namespace qoala::analysis::precedences {
    /**
     * Track precedences (data dependencies, predecessors, communication/entanglement ordering).
     * Make the precedences information available by inserting a "blk_meta"
     * operation At the beginning of each block.
     * @param moduleOp module to walk for tracking and adding precedences.
     * @param useOnlineScheduler whether to use online scheduler.
     * @return an MLIR value success or failure.
     */

    mlir::LogicalResult addPrecedences(mlir::ModuleOp &moduleOp, bool useOnlineScheduler = false);
} // namespace precedences
#endif // QOALA_MLIR_PRECEDENCES_H
