#include "Analysis/Helpers/Helpers.h"
#include "Dialect/Helpers/DialectHelpers.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "mlir/Interfaces/FunctionImplementation.h"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::helpers;

#include "Dialect/NetQASM/NetQASM.h"
// include generated source code for operations
#define GET_OP_CLASSES
#include "Dialect/QoalaHost/QoalaHost.cpp.inc"

// include generated source code for types
#define GET_TYPEDEF_CLASSES
#include "Dialect/QoalaHost/QoalaHostTypes.cpp.inc"


/* Parse and print functions "ported" from func.func: parse and print */
ParseResult qoalahost::MainFuncOp::parse(OpAsmParser &parser, OperationState &result) {
    auto buildFuncType =
            [](Builder &builder, ArrayRef<Type> argTypes, ArrayRef<Type> results,
               function_interface_impl::VariadicFlag,
               std::string &) { return builder.getFunctionType(argTypes, results); };

    return function_interface_impl::parseFunctionOp(
            parser, result, /*allowVariadic=*/false,
            getFunctionTypeAttrName(result.name), buildFuncType,
            getArgAttrsAttrName(result.name), getResAttrsAttrName(result.name));
}


void qoalahost::MainFuncOp::build(OpBuilder &builder, OperationState &state, StringRef name,
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

void qoalahost::MainFuncOp::print(OpAsmPrinter &p) {
    function_interface_impl::printFunctionOp(
            p, *this, /*isVariadic=*/false, getFunctionTypeAttrName(),
            getArgAttrsAttrName(), getResAttrsAttrName());
}

/* Region verifiers for MainFuncOp */
LogicalResult qoalahost::MainFuncOp::verifyRegions() {
    for (Operation &operation : this->getBody().getOps()) {
        auto name = operation.getName().getStringRef().str();
        if (QoalaHostDialect::opIsNotFromAllowedDialects(operation)) {
            return this->emitOpError() << "'" << getOperationName() << "' "
                                          << "op contains an operation that is not from  the allowed list of dialects: ["
                                          << QoalaHostDialect::getAllowedDialectNames()
                                          << "]. Operation: '" << operation << "'";
        }
    }
    return success();
}

/* Call operation verifier */
LogicalResult qoalahost::CallOp::verifySymbolUses(SymbolTableCollection &symbolTable) {
    // Search the symbol in the parent SymbolTables
    // The declared symbol MUST come from a netqasm.local_routine OR netqasm.request_routine operation
    if (symbolTable.lookupNearestSymbolFrom<netqasm::LocalRoutineOp>(this->getOperation(), this->getCalleeAttr())) {
        return success();
    }
    if (symbolTable.lookupNearestSymbolFrom<netqasm::RequestRoutineOp>(this->getOperation(), this->getCalleeAttr())) {
        return success();
    }
    return this->emitOpError() << "'" << this->getCalleeAttr().getAttr().str() << "' "
                               << "does not reference a valid defined by either netqasm.local_routine or "
                               << "netqasm.request_routine.";
}

/* Additional verifier for the Call operation to check number of arguments used. */
LogicalResult qoalahost::CallOp::verify() {
    auto module = this->getOperation()->getParentOfType<ModuleOp>();
    // Even if we write MLIR without declaring a module, MLIR will insert a top-level module operation, so
    // it is safe to assert the existence of the module.
    assert(module && "Unexpected - Operation not inside an MLIR module operation");
    if (const Operation *callee = helpers::getRoutineWithName(&module, this->getCallee()); !callee) {
        this->emitOpError() << "Called function '" << this->getCallee() << "' was not found in the module.";
    } else {
        auto netQASMRoutine = dyn_cast<NetQASMRoutineInterface>(callee);
        if (this->getArgOperands().size() != netQASMRoutine.getArgsTypesList().size()) {
            this->emitError() << "Call operation does not match the number of arguments of the callee.";
        }
    }
    return success();
}

/* Helper functions from the QoalaHostDialect class */
bool qoalahost::QoalaHostDialect::opIsNotFromAllowedDialects(Operation &operation) {
    return !belongsToDialect<
#define GET_ALLOWED_DIALECTS
#include "Dialect/QoalaHost/QoalaHost.h"
    >(operation);
}

std::string qoalahost::QoalaHostDialect::getAllowedDialectNames() {
    return getDialectNamesList<
#define GET_ALLOWED_DIALECTS
#include "Dialect/QoalaHost/QoalaHost.h"
    >();
}
