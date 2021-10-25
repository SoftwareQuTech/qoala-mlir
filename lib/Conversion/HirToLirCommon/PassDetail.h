#ifndef CONVERSION_HIRTOLIR_PASSDETAIL_H_
#define CONVERSION_HIRTOLIR_PASSDETAIL_H_

#include "mlir/Pass/Pass.h"

namespace mlir
{

#define GEN_PASS_CLASSES
#include "Conversion/HirToLirCommon/Passes.h.inc"

} // end namespace mlir

#endif // CONVERSION_HIRTOLIR_PASSDETAIL_H_
