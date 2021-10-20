#include "hir/Dialect.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/OpImplementation.h"

using namespace mlir;
using namespace mlir::hir;

#include "hir/Dialect.cpp.inc"

/// Dialect initialization, the instance will be owned by the context. This is
/// the point of registration of types and operations for the dialect.
void HirDialect::initialize()
{
  addOperations<
#define GET_OP_LIST
#include "hir/Ops.cpp.inc"
      >();
}

static mlir::ParseResult parseConstantOp(mlir::OpAsmParser &parser,
                                         mlir::OperationState &result)
{
  mlir::DenseElementsAttr value;
  if (parser.parseOptionalAttrDict(result.attributes) ||
      parser.parseAttribute(value, "value", result.attributes))
    return failure();

  result.addTypes(value.getType());
  return success();
}

// mlir::Type HirDialect::parseType(mlir::DialectAsmParser &parser) const
// {
//   return Type();
// }

// void HirDialect::printType(mlir::Type type, mlir::DialectAsmPrinter &printer) const
// {
//   printer << "qubit";
// }

static void print(mlir::OpAsmPrinter &printer, ConstantOp op)
{
  printer << " ";
  printer.printOptionalAttrDict(op->getAttrs(), /*elidedAttrs=*/{"value"});
  printer << op.value();
}

static mlir::LogicalResult verify(ConstantOp op)
{
  // If the return type of the constant is not an unranked tensor, the shape
  // must match the shape of the attribute holding the data.
  auto resultType = op.getResult().getType().dyn_cast<mlir::RankedTensorType>();
  if (!resultType)
    return success();

  // Check that the rank of the attribute type matches the rank of the constant
  // result type.
  auto attrType = op.value().getType().cast<mlir::TensorType>();
  if (attrType.getRank() != resultType.getRank())
  {
    return op.emitOpError(
               "return type must match the one of the attached value "
               "attribute: ")
           << attrType.getRank() << " != " << resultType.getRank();
  }

  // Check that each of the dimensions match between the two types.
  for (int dim = 0, dimE = attrType.getRank(); dim < dimE; ++dim)
  {
    if (attrType.getShape()[dim] != resultType.getShape()[dim])
    {
      return op.emitOpError(
                 "return type shape mismatches its attribute at dimension ")
             << dim << ": " << attrType.getShape()[dim]
             << " != " << resultType.getShape()[dim];
    }
  }
  return mlir::success();
}

//===----------------------------------------------------------------------===//
// TableGen'd op method definitions
//===----------------------------------------------------------------------===//

#define GET_OP_CLASSES
#include "hir/Ops.cpp.inc"
