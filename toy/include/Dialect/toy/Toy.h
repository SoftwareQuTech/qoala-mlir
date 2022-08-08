#ifndef TOY_TOY_H
#define TOY_TOY_H

#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/Support/LogicalResult.h"

#define GET_OP_CLASSES
#include "Dialect/toy/Toy.h.inc"

using namespace mlir;

class ConstantOp : public mlir::Op<
                       ConstantOp,
                       mlir::OpTrait::ZeroOperands,
                       mlir::OpTrait::OneResult,
                       mlir::OpTrait::OneTypedResult<TensorType>::Impl>
{
    using Op::Op;

public:
    static llvm::StringRef getOperationName() { return "toy.constant"; }

    mlir::DenseElementsAttr getValue();

    LogicalResult verifyInvariants();

    static void build(mlir::OpBuilder &builder, mlir::OperationState &state,
                      mlir::Type result, mlir::DenseElementsAttr value);
    /// Build a constant and reuse the type from the given 'value'.
    static void build(mlir::OpBuilder &builder, mlir::OperationState &state,
                      mlir::DenseElementsAttr value);
    /// Build a constant by broadcasting the given 'value'.
    static void build(mlir::OpBuilder &builder, mlir::OperationState &state,
                      double value);
};

#endif // TOY_TOY_H