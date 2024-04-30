#include "Analysis/Helpers/Helpers.h"
#include <string>

using namespace mlir;
using namespace qoala::helpers;

/* Region verifiers for MainFuncOp */
std::string qoala::helpers::getAllowedDialectNames() {
    std::string result;
    result += "'" + arith::ArithDialect::getDialectNamespace().str() + "', '"
                  + memref::MemRefDialect::getDialectNamespace().str() + "', '"
                  + cf::ControlFlowDialect::getDialectNamespace().str() + "', '"
                  + tensor::TensorDialect::getDialectNamespace().str() + "', '"
                  + affine::AffineDialect::getDialectNamespace().str() + "' ";
    return result;
}


/* Implementation of the null types converter */
qoala::helpers::NullTypeConverter::NullTypeConverter(MLIRContext *ctx) {
    // For all types, we map it to the same instance
    this->addConversion([](Type type) { return type; });
}
