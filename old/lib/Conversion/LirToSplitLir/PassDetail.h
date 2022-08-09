#ifndef CONVERSION_LIRTOSPLITLIT_PASSDETAIL_H_
#define CONVERSION_LIRTOSPLITLIT_PASSDETAIL_H_

#include "mlir/Pass/Pass.h"

namespace mlir
{

#define GEN_PASS_CLASSES
#include "Conversion/LirToSplitLir/Passes.h.inc"

} // end namespace mlir

#endif // CONVERSION_LIRTOSPLITLIT_PASSDETAIL_H_
