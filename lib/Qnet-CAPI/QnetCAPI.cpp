#include "Qnet-C/Qnet-c.h"
#include "mlir-c/Support.h"

#include "Dialect/Qnet/QnetDialect.h"
#include "Dialect/Qnet/QnetTypes.h"
#include "mlir/CAPI/Registration.h"

MLIR_DEFINE_CAPI_DIALECT_REGISTRATION(Qnet, qnet,
                                      qoala::dialects::qnet::QnetDialect)

bool mlirTypeIsAQubitType(MlirType type) {
    return mlir::isa<qoala::dialects::qnet::QubitType>(unwrap(type));
}

MlirType mlirQubitTypeGet(MlirContext ctx) {
    return wrap(qoala::dialects::qnet::QubitType::get(unwrap(ctx)));
}