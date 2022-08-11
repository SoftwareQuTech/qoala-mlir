#include "mlir/IR/Attributes.h"
#include "mlir/IR/BlockAndValueMapping.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Support/LLVM.h"
#include "llvm/ADT/ArrayRef.h"
#include "mlir/IR/FunctionImplementation.h"

#include "Dialect/qoala_lir/QoalaLir.h"
#include "Dialect/qoala_lir/QoalaLirDialect.h"

using namespace mlir;
using namespace mlir::qoala_lir;

#define GET_OP_CLASSES

void ShapeFuncOp::print(OpAsmPrinter &p)
{
    function_interface_impl::printFunctionOp(p, *this, /*isVariadic=*/false);
}

ParseResult ShapeFuncOp::parse(OpAsmParser &parser, OperationState &result)
{
    auto buildFuncType =
        [](Builder &builder, ArrayRef<Type> argTypes, ArrayRef<Type> results,
           function_interface_impl::VariadicFlag,
           std::string &)
    { return builder.getFunctionType(argTypes, results); };

    return function_interface_impl::parseFunctionOp(
        parser, result, /*allowVariadic=*/false, buildFuncType);
}

#include "Dialect/qoala_lir/QoalaLir.cpp.inc"
