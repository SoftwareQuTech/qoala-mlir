#include "Analysis/Helpers/Helpers.h"
#include "llvm/Support/raw_ostream.h"
#include "mlir/Interfaces/FunctionImplementation.h"

#include "Dialect/NetQASM/NetQASM.h"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::helpers;

#include "Dialect/QoalaHost/QoalaHost.h"
// include generated source code for operations
#define GET_OP_CLASSES
#include "Dialect/NetQASM/NetQASM.cpp.inc"

// include generated "dispatcher" of the operation interface
#include "Analysis/Helpers/NetQASMInterfaces.cpp.inc"

/* Parse and print functions "ported" from func.func: parse, print and build */
ParseResult netqasm::LocalRoutineOp::parse(OpAsmParser &parser, OperationState &result) {
    auto buildFuncType = [](Builder &builder, ArrayRef<Type> argTypes, ArrayRef<Type> results,
                            function_interface_impl::VariadicFlag,
                            std::string &) { return builder.getFunctionType(argTypes, results); };

    return function_interface_impl::parseFunctionOp(parser, result, /*allowVariadic=*/false,
                                                    getFunctionTypeAttrName(result.name), buildFuncType,
                                                    getArgAttrsAttrName(result.name), getResAttrsAttrName(result.name));
}

void netqasm::LocalRoutineOp::print(OpAsmPrinter &p) {
    function_interface_impl::printFunctionOp(p, *this, /*isVariadic=*/false, getFunctionTypeAttrName(),
                                             getArgAttrsAttrName(), getResAttrsAttrName());
}

void netqasm::LocalRoutineOp::build(OpBuilder &builder, OperationState &state, StringRef name, FunctionType type,
                                    ArrayRef<NamedAttribute> attrs, ArrayRef<DictionaryAttr> argAttrs) {
    state.addAttribute(SymbolTable::getSymbolAttrName(), builder.getStringAttr(name));
    state.addAttribute(getFunctionTypeAttrName(state.name), TypeAttr::get(type));
    state.attributes.append(attrs.begin(), attrs.end());
    state.addRegion();

    if (argAttrs.empty()) {
        return;
    }
    assert(type.getNumInputs() == argAttrs.size());
    function_interface_impl::addArgAndResultAttrs(builder, state, argAttrs, /*resultAttrs=*/std::nullopt,
                                                  getArgAttrsAttrName(state.name), getResAttrsAttrName(state.name));
}

/* Parse and print functions "ported" from func.func: parse and print */
ParseResult netqasm::RequestRoutineOp::parse(OpAsmParser &parser, OperationState &result) {
    auto buildFuncType = [](Builder &builder, ArrayRef<Type> argTypes, ArrayRef<Type> results,
                            function_interface_impl::VariadicFlag,
                            std::string &) { return builder.getFunctionType(argTypes, results); };

    return function_interface_impl::parseFunctionOp(parser, result, /*allowVariadic=*/false,
                                                    getFunctionTypeAttrName(result.name), buildFuncType,
                                                    getArgAttrsAttrName(result.name), getResAttrsAttrName(result.name));
}

void netqasm::RequestRoutineOp::print(OpAsmPrinter &p) {
    function_interface_impl::printFunctionOp(p, *this, /*isVariadic=*/false, getFunctionTypeAttrName(),
                                             getArgAttrsAttrName(), getResAttrsAttrName());
}

Operation *netqasm::LocalRoutineOp::getReturnOperation() {
    const auto returnOps = this->getOps<ReturnOp>();
    assert(!returnOps.empty() && "Local routine must have at least one return operation");
    return *returnOps.begin();
}

Operation *netqasm::RequestRoutineOp::getReturnOperation() {
    const auto returnOps = this->getOps<ReturnOp>();
    assert(!returnOps.empty() && "Local routine must have at least one return operation");
    return *returnOps.begin();
}

MutableArrayRef<BlockArgument> netqasm::RequestRoutineOp::getArgsTypesList() { return this->getArguments(); }

MutableArrayRef<BlockArgument> netqasm::LocalRoutineOp::getArgsTypesList() { return this->getArguments(); }

/* Helper functions from the NetQASMDialect class */
bool netqasm::NetQASMDialect::opIsNotFromAllowedDialects(Operation &operation) {
    return !belongsToDialect<
#define GET_ALLOWED_DIALECTS
#include "Dialect/NetQASM/NetQASM.h"
            >(operation);
}

uint32_t netqasm::QAllocOp::getDuration() { return options::qoalaOptQNosInstrTime; }

uint32_t netqasm::QFreeOp::getDuration() { return options::qoalaOptQNosInstrTime; }

uint32_t netqasm::ReturnOp::getDuration() { return this->getNumOperands() * options::qoalaOptQNosInstrTime; }

uint32_t netqasm::QInitOp::getDuration() { return options::qoalaOptSingleGateDuration; }

uint32_t netqasm::RotateXOp::getDuration() { return options::qoalaOptSingleGateDuration; }

uint32_t netqasm::RotateYOp::getDuration() { return options::qoalaOptSingleGateDuration; }

uint32_t netqasm::RotateZOp::getDuration() { return options::qoalaOptSingleGateDuration; }

uint32_t netqasm::HadamardOp::getDuration() { return options::qoalaOptSingleGateDuration; }

uint32_t netqasm::MeasureOp::getDuration() { return options::qoalaOptSingleGateDuration; }

uint32_t netqasm::CnotOp::getDuration() { return options::qoalaOptTwoGateDuration; }

uint32_t netqasm::CzOp::getDuration() { return options::qoalaOptTwoGateDuration; }

uint32_t netqasm::CrotXOp::getDuration() { return options::qoalaOptTwoGateDuration; }

uint32_t netqasm::EprsOp::getDuration() { return options::qoalaOptLinkDuration; }

uint32_t netqasm::EprsMeasureOp::getDuration() {
    return options::qoalaOptLinkDuration + options::qoalaOptSingleGateDuration;
}

StringRef netqasm::EprsMeasureOp::getUsedRemoteName() { return this->getRemote(); }

FlatSymbolRefAttr netqasm::EprsMeasureOp::getUsedRemoteAttr() { return this->getRemoteAttr(); }

StringRef netqasm::EprsOp::getUsedRemoteName() { return this->getRemote(); }

FlatSymbolRefAttr netqasm::EprsOp::getUsedRemoteAttr() { return this->getRemoteAttr(); }

std::string netqasm::NetQASMDialect::getAllowedDialectNames() {
    return getDialectNamesList<
#define GET_ALLOWED_DIALECTS
#include "Dialect/NetQASM/NetQASM.h"
            >();
}
