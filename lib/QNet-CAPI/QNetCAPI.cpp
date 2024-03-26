#include "QNet-C/QNet-c.h"
#include "mlir-c/Support.h"

#include "Dialect/QNet/QNetDialect.h"
#include "Dialect/QNet/QNetTypes.h"
#include "mlir/CAPI/Registration.h"

MLIR_DEFINE_CAPI_DIALECT_REGISTRATION(QNet, qnet,
                                      qoala::dialects::qnet::QNetDialect)

bool mlirTypeIsAQubitType(MlirType type) {
    return mlir::isa<qoala::dialects::qnet::QubitType>(unwrap(type));
}

MlirType mlirQubitTypeGet(MlirContext ctx) {
    return wrap(qoala::dialects::qnet::QubitType::get(unwrap(ctx)));
}