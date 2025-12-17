#ifndef QOALA_MLIR_REMOTEIDS_H
#define QOALA_MLIR_REMOTEIDS_H

#include "mlir/IR/BuiltinOps.h"
#include "mlir/Support/LogicalResult.h"

namespace qoala::analysis::remoteids {
    mlir::LogicalResult addRemoteIDs(mlir::ModuleOp &module);
} // namespace qoala::analysis::remoteids

#endif // QOALA_MLIR_REMOTEIDS_H
