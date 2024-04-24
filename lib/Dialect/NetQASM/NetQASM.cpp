#include "mlir/Interfaces/FunctionImplementation.h"
#include "llvm/Support/raw_ostream.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"

#include "Dialect/NetQASM/NetQASM.h"

using namespace qoala::dialects::netqasm;

// include generated source code for operations
#define GET_OP_CLASSES
#include "Dialect/NetQASM/NetQASM.cpp.inc"

/* Parse and print functions "ported" from func.func: parse and print */
ParseResult LocalRoutineOp::parse(OpAsmParser &parser, OperationState &result) {
    auto buildFuncType =
            [](Builder &builder, ArrayRef<Type> argTypes, ArrayRef<Type> results,
               function_interface_impl::VariadicFlag,
               std::string &) { return builder.getFunctionType(argTypes, results); };

    return function_interface_impl::parseFunctionOp(
            parser, result, /*allowVariadic=*/false,
            getFunctionTypeAttrName(result.name), buildFuncType,
            getArgAttrsAttrName(result.name), getResAttrsAttrName(result.name));
}

void LocalRoutineOp::print(OpAsmPrinter &p) {
    function_interface_impl::printFunctionOp(
            p, *this, /*isVariadic=*/false, getFunctionTypeAttrName(),
            getArgAttrsAttrName(), getResAttrsAttrName());
}

/* Parse and print functions "ported" from func.func: parse and print */
ParseResult RequestRoutineOp::parse(OpAsmParser &parser, OperationState &result) {
    auto buildFuncType =
            [](Builder &builder, ArrayRef<Type> argTypes, ArrayRef<Type> results,
               function_interface_impl::VariadicFlag,
               std::string &) { return builder.getFunctionType(argTypes, results); };

    return function_interface_impl::parseFunctionOp(
            parser, result, /*allowVariadic=*/false,
            getFunctionTypeAttrName(result.name), buildFuncType,
            getArgAttrsAttrName(result.name), getResAttrsAttrName(result.name));
}

void RequestRoutineOp::print(OpAsmPrinter &p) {
    function_interface_impl::printFunctionOp(
            p, *this, /*isVariadic=*/false, getFunctionTypeAttrName(),
            getArgAttrsAttrName(), getResAttrsAttrName());
}

/* Region verifiers for LocalRoutineOp and RequestRoutineOp */
static bool operationIsNotValid(Operation &operation) {
    return ! (isa<
#define GET_OP_LIST
#include "mlir/Dialect/Arith/IR/ArithOps.cpp.inc"
            >(operation) ||
            isa<
#define GET_OP_LIST
#include "mlir/Dialect/MemRef/IR/MemRefOps.cpp.inc"
            >(operation) ||
            isa<
#define GET_OP_LIST
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.cpp.inc"
            >(operation));
}

LogicalResult RequestRoutineOp::verifyRegions() {
    Region &region = getBody();
    for (Block &block : region) {
        for (Operation &operation : block) {
            if (operationIsNotValid(operation)) {
                std::string message;
                llvm::raw_string_ostream formatter(message);
                formatter << getOperationName() << "op contains an operation that is not from 'arith', 'memref' "
                                                   "or 'cf' dialects: '" << operation << "'";
                formatter.flush();
                return emitError(message);
            }
        }
    }
    return success();
}

LogicalResult LocalRoutineOp::verifyRegions() {
    Region &region = getBody();
    for (Block &block : region) {
        for (Operation &operation : block) {
            if (operationIsNotValid(operation)) {
                std::string message;
                llvm::raw_string_ostream formatter(message);
                formatter << getOperationName() << "op contains an operation that is not from 'arith', 'memref' "
                                                   "or 'cf' dialects: '" << operation << "'";
                formatter.flush();
                return emitError(message);
            }
        }
    }
    return success();
}