#include "mlir/Interfaces/FunctionImplementation.h"
#include "llvm/Support/raw_ostream.h"
#include "Analysis/Helpers/Helpers.h"

#include "Dialect/NetQASM/NetQASM.h"

using namespace qoala::dialects;
using namespace qoala::helpers;

#include "Dialect/QoalaHost/QoalaHost.h"
// include generated source code for operations
#define GET_OP_CLASSES
#include "Dialect/NetQASM/NetQASM.cpp.inc"

/* Parse and print functions "ported" from func.func: parse and print */
ParseResult netqasm::LocalRoutineOp::parse(OpAsmParser &parser, OperationState &result) {
    auto buildFuncType =
            [](Builder &builder, ArrayRef<Type> argTypes, ArrayRef<Type> results,
               function_interface_impl::VariadicFlag,
               std::string &) { return builder.getFunctionType(argTypes, results); };

    return function_interface_impl::parseFunctionOp(
            parser, result, /*allowVariadic=*/false,
            getFunctionTypeAttrName(result.name), buildFuncType,
            getArgAttrsAttrName(result.name), getResAttrsAttrName(result.name));
}

void netqasm::LocalRoutineOp::print(OpAsmPrinter &p) {
    function_interface_impl::printFunctionOp(
            p, *this, /*isVariadic=*/false, getFunctionTypeAttrName(),
            getArgAttrsAttrName(), getResAttrsAttrName());
}

/* Parse and print functions "ported" from func.func: parse and print */
ParseResult netqasm::RequestRoutineOp::parse(OpAsmParser &parser, OperationState &result) {
    auto buildFuncType =
            [](Builder &builder, ArrayRef<Type> argTypes, ArrayRef<Type> results,
               function_interface_impl::VariadicFlag,
               std::string &) { return builder.getFunctionType(argTypes, results); };

    return function_interface_impl::parseFunctionOp(
            parser, result, /*allowVariadic=*/false,
            getFunctionTypeAttrName(result.name), buildFuncType,
            getArgAttrsAttrName(result.name), getResAttrsAttrName(result.name));
}

void netqasm::RequestRoutineOp::print(OpAsmPrinter &p) {
    function_interface_impl::printFunctionOp(
            p, *this, /*isVariadic=*/false, getFunctionTypeAttrName(),
            getArgAttrsAttrName(), getResAttrsAttrName());
}

/* Helper functions from the NetQASMDialect class */
bool netqasm::NetQASMDialect::opIsNotFromAllowedDialects(Operation &operation) {
    return !belongsToDialect<
#define GET_ALLOWED_DIALECTS
#include "Dialect/NetQASM/NetQASM.h"
    >(operation);
}

std::string netqasm::NetQASMDialect::getAllowedDialectNames() {
    return getDialectNamesList<
#define GET_ALLOWED_DIALECTS
#include "Dialect/NetQASM/NetQASM.h"
    >();
}
