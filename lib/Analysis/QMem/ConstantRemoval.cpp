#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "Dialect/QMem/Passes.h"
#include "Dialect/QMem/QMem.h"

#include "Analysis/QMem/Conversion.h"
#include "Analysis/Helpers/Helpers.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIRPatterns.h"

#include "llvm/Support/Debug.h"

namespace mlir {
} // namespace mlir

using namespace mlir;
using namespace qoala::helpers;

#define DEBUG_TYPE "const-removal"

namespace qoala::helpers {
    LogicalResult removeOrphanConstants(mlir::ModuleOp &module) {
        // TODO
        return success();
    }
} /* namespace qoala::analysis */