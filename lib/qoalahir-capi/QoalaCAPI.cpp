#include "qoalahir-c/qoalahir.h"
#include "mlir-c/Support.h"

#include "Dialect/hir/HirDialect.h"
#include "mlir/CAPI/Registration.h"
#include "Dialect/hir/HirTypes.h"

MLIR_DEFINE_CAPI_DIALECT_REGISTRATION(Hir, hir, mlir::hir::HirDialect)

bool mlirTypeIsAQubitType(MlirType type) {
    return mlir::isa<mlir::hir::QubitType>(unwrap(type));
}

MlirType mlirQubitTypeGet(MlirContext ctx) {
    return wrap(mlir::hir::QubitType::get(unwrap(ctx)));
}