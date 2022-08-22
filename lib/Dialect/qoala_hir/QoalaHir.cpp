#include "mlir/IR/Attributes.h"
#include "mlir/IR/BlockAndValueMapping.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Support/LLVM.h"
#include "llvm/ADT/ArrayRef.h"

#include "Dialect/qoala_hir/QoalaHir.h"
#include "Dialect/qoala_hir/QoalaHirDialect.h"
#include "Dialect/qoala_hir/QoalaHirTypes.h"

using namespace mlir;
using namespace mlir::qoala_hir;

#define GET_OP_CLASSES
#include "Dialect/qoala_hir/QoalaHir.cpp.inc"
