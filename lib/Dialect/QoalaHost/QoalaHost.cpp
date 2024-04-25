#include "Dialect/QoalaHost/QoalaHost.h"
#include "Analysis/Helpers/Helpers.h"
#include "mlir/Interfaces/FunctionImplementation.h"

using namespace mlir;
using namespace qoala::dialects::qoalahost;
using namespace qoala::helpers;

#include "Dialect/NetQASM/NetQASM.h"
// include generated source code for operations
#define GET_OP_CLASSES
#include "Dialect/QoalaHost/QoalaHost.cpp.inc"

// include generated source code for types
#define GET_TYPEDEF_CLASSES
#include "Dialect/QoalaHost/QoalaHostTypes.cpp.inc"


/* Parse and print functions "ported" from func.func: parse and print */
ParseResult MainFuncOp::parse(OpAsmParser &parser, OperationState &result) {
    auto buildFuncType =
            [](Builder &builder, ArrayRef<Type> argTypes, ArrayRef<Type> results,
               function_interface_impl::VariadicFlag,
               std::string &) { return builder.getFunctionType(argTypes, results); };

    return function_interface_impl::parseFunctionOp(
            parser, result, /*allowVariadic=*/false,
            getFunctionTypeAttrName(result.name), buildFuncType,
            getArgAttrsAttrName(result.name), getResAttrsAttrName(result.name));
}


void MainFuncOp::build(OpBuilder &builder, OperationState &state, StringRef name,
                       FunctionType type, ArrayRef<NamedAttribute> attrs,
                       ArrayRef<DictionaryAttr> argAttrs) {
    state.addAttribute(mlir::SymbolTable::getSymbolAttrName(),
                       builder.getStringAttr(name));
    state.addAttribute(getFunctionTypeAttrName(state.name), TypeAttr::get(type));
    state.attributes.append(attrs.begin(), attrs.end());
    state.addRegion();

    if (argAttrs.empty())
        return;
    assert(type.getNumInputs() == argAttrs.size());
    function_interface_impl::addArgAndResultAttrs(
            builder, state, argAttrs, /*resultAttrs=*/std::nullopt,
            getArgAttrsAttrName(state.name), getResAttrsAttrName(state.name));
}

void MainFuncOp::print(OpAsmPrinter &p) {
    function_interface_impl::printFunctionOp(
            p, *this, /*isVariadic=*/false, getFunctionTypeAttrName(),
            getArgAttrsAttrName(), getResAttrsAttrName());
}

/* Region verifiers for MainFuncOp */
static bool operationIsNotFromQoalaHost(Operation &operation) {
    return ! (isa<
#define GET_OP_LIST
#include "Dialect/QoalaHost/QoalaHost.cpp.inc"
              >(operation));
}

LogicalResult MainFuncOp::verifyRegions() {
    Region &region = getBody();
    for (Block &block : region) {
        for (Operation &operation : block) {
            if (operationIsNotFromArithMemRefOrCFDialects(operation) || operationIsNotFromQoalaHost(operation)) {
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

/* Call operation verifier */
LogicalResult CallOp::verifySymbolUses(mlir::SymbolTableCollection &symbolTable) {
    // Check that the callee attribute was specified.
    auto fnAttr = (*this)->getAttrOfType<FlatSymbolRefAttr>("callee");
    if (!fnAttr)
        return this->emitOpError("requires a 'remote' symbol reference attribute");
    // Search the symbol in the parent SymbolTables
    // The declared symbol MUST come from a netqasm.local_routine OR netqasm.request_routine operation
    if (symbolTable.lookupNearestSymbolFrom<netqasm::LocalRoutineOp>(this->getOperation(), this->getCalleeAttr())) {
        return success();
    }
    if (symbolTable.lookupNearestSymbolFrom<netqasm::RequestRoutineOp>(this->getOperation(), this->getCalleeAttr())) {
        return success();
    }
    return this->emitOpError() << "'" << fnAttr.getValue()
                               << "' does not reference a valid remote node";
}
