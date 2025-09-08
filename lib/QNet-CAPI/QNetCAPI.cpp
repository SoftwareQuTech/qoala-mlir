#include "QNet-C/QNet-c.h"
#include "mlir-c/Support.h"

#include "Dialect/QNet/QNetDialect.h"
#include "Dialect/QNet/QNetTypes.h"
#include "mlir/CAPI/Registration.h"

using namespace qoala::dialects;

MLIR_DEFINE_CAPI_DIALECT_REGISTRATION(QNet, qnet, qnet::QNetDialect)

bool mlirTypeIsAQubitType(MlirType type) { return mlir::isa<qnet::QubitType>(unwrap(type)); }

MlirType mlirQubitTypeGet(MlirContext ctx) { return wrap(qnet::QubitType::get(unwrap(ctx))); }

MlirTypeID mlirQubitTypeGetTypeID() { return wrap(qnet::QubitType::getTypeID()); }
