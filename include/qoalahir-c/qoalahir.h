#ifndef QOALA_MLIR_QOALAHIR_H
#define QOALA_MLIR_QOALAHIR_H

#include "mlir-c/IR.h"

#ifdef __cplusplus
extern "C" {
#endif

MLIR_DECLARE_CAPI_DIALECT_REGISTRATION(Hir, hir);

MLIR_CAPI_EXPORTED bool mlirTypeIsAQubitType(MlirType type);

MLIR_CAPI_EXPORTED MlirType mlirQubitTypeGet(MlirContext ctx);

#ifdef __cplusplus
}
#endif

#endif //QOALA_MLIR_QOALAHIR_H
