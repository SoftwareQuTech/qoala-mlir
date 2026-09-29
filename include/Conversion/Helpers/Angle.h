#ifndef QOALA_MLIR_ANGLE_H
#define QOALA_MLIR_ANGLE_H

#include "mlir/IR/BuiltinOps.h"

namespace qoala::helpers::angle {
    extern std::string angleConversionFunctionName;

    bool moduleContainsAngleConversionDeclaration(mlir::ModuleOp &module);
    mlir::Operation *insertAngleConversionFunctionDeclaration(mlir::ModuleOp &module);
    std::vector<uint32_t> transformDouble(double angleRads);
} // namespace qoala::helpers::angle

#endif // QOALA_MLIR_ANGLE_H
