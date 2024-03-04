#include "qoalahir-c/qoalahir.h"

#include "Dialect/hir/HirDialect.h"
#include "mlir/CAPI/Registration.h"

MLIR_DEFINE_CAPI_DIALECT_REGISTRATION(Hir, hir, mlir::hir::HirDialect)