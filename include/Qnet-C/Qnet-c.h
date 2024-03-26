#ifndef QNET_C_H
#define QNET_C_H

#include "mlir-c/IR.h"

#ifdef __cplusplus
extern "C" {
#endif

MLIR_DECLARE_CAPI_DIALECT_REGISTRATION(Qnet, qnet);

MLIR_CAPI_EXPORTED bool mlirTypeIsAQubitType(MlirType type);

MLIR_CAPI_EXPORTED MlirType mlirQubitTypeGet(MlirContext ctx);

#ifdef __cplusplus
}
#endif

#endif // QNET_C_H
