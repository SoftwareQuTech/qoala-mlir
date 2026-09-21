#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Debug.h"
#include "mlir/Interfaces/FunctionImplementation.h"

#include "Analysis/Helpers/Helpers.h"
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

#define DEBUG_TYPE "NetQASM ops"

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

std::optional<Operation *> netqasm::LocalRoutineOp::getReturnOperation() {
    const auto returnOps = this->getOps<ReturnOp>();
    if (!returnOps.empty()) {
        return *returnOps.begin();
    }
    return std::nullopt;
}

std::optional<Operation *> netqasm::RequestRoutineOp::getReturnOperation() {
    const auto returnOps = this->getOps<ReturnOp>();
    assert(!returnOps.empty() && "Request routine must have at least one return operation");
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

uint64_t netqasm::QAllocOp::getDuration() { return options::qoalaOptQNosInstrTime; }

uint64_t netqasm::QFreeOp::getDuration() { return options::qoalaOptQNosInstrTime; }

uint64_t netqasm::ReturnOp::getDuration() { return this->getNumOperands() * options::qoalaOptQNosInstrTime; }

uint64_t netqasm::QInitOp::getDuration() { return options::qoalaOptSingleGateDuration; }

uint64_t netqasm::RotateXOp::getDuration() { return options::qoalaOptSingleGateDuration; }

uint64_t netqasm::RotateYOp::getDuration() { return options::qoalaOptSingleGateDuration; }

uint64_t netqasm::RotateZOp::getDuration() { return options::qoalaOptSingleGateDuration; }

uint64_t netqasm::HadamardOp::getDuration() { return options::qoalaOptSingleGateDuration; }

uint64_t netqasm::XOp::getDuration() { return options::qoalaOptSingleGateDuration; }

uint64_t netqasm::YOp::getDuration() { return options::qoalaOptSingleGateDuration; }

uint64_t netqasm::ZOp::getDuration() { return options::qoalaOptSingleGateDuration; }

uint64_t netqasm::MeasureOp::getDuration() { return options::qoalaOptSingleGateDuration; }

uint64_t netqasm::CnotOp::getDuration() { return options::qoalaOptTwoGateDuration; }

uint64_t netqasm::CzOp::getDuration() { return options::qoalaOptTwoGateDuration; }

uint64_t netqasm::CrotXOp::getDuration() { return options::qoalaOptTwoGateDuration; }

uint64_t netqasm::EprsOp::getDuration() { return options::qoalaOptLinkDuration; }

uint64_t netqasm::EprsMeasureOp::getDuration() {
    return options::qoalaOptLinkDuration + options::qoalaOptSingleGateDuration;
}

static bool containsEPRSOperation(Operation *op) {
    bool hasEPRS = false;
    bool hasEPRSMeasure = false;
    op->walk([&hasEPRS](netqasm::EprsMeasureOp eprsOp) {
        hasEPRS = true;
        return WalkResult::interrupt();
    });
    op->walk([&hasEPRSMeasure](netqasm::EprsOp eprsOp) {
        hasEPRSMeasure = true;
        return WalkResult::interrupt();
    });
    return hasEPRS || hasEPRSMeasure;
}

LogicalResult netqasm::RequestRoutineOp::validateNestedInstructions() {
    // Request routines *must* contain an EPRS-related operation
    if (!containsEPRSOperation(this->getOperation())) {
        this->emitError("Request routines must contain an EPRS-related operation.");
        return failure();
    }

    // Request routines return value must complain with one of the following criteria:
    // * Return an i1. If so, the i1 value *must* trace back directly to an eprs_measure operation.
    //   This is the case of a "create_measure" type of request routine.
    // * Return an i32. If so, the i32 value *must* be used directly by an eprs operation.
    //   This is the case of a "create_keep" type of request routine.

    auto returnOp = dyn_cast<ReturnOp>(this->getReturnOperation());
    if (!returnOp) {
        this->emitError() << "Request routine '" << this->getName() << "' does not return a value.";
        return failure();
    }
    for (Value retVal : returnOp.getOperands()) {
        if (retVal.getType().isInteger(1)) {
            if (!isa<EprsMeasureOp>(retVal.getDefiningOp())) {
                returnOp.emitError() << "Returned value '" << retVal << "' is an i1 and does not come from "
                                     << "a netqasm.eprs_measure operation.";
            }
            continue;
        }
        if (retVal.getType().isInteger(32)) {
            if (auto allocOp = dyn_cast<QAllocOp>(retVal.getDefiningOp()); !allocOp) {
                returnOp.emitError() << "Returned value '" << retVal << "' is an i32 and it was not created "
                                     << "by netqasm.qalloc operation.";
            } else {
                auto userOps = allocOp->getUsers();
                assert(!userOps.empty() && "Qubit users are empty - This should not be the case");
                bool entangled = std::any_of(userOps.begin(), userOps.end(),
                                             [](Operation *userOp) { return isa<EprsOp>(userOp); });
                if (!entangled) {
                    returnOp.emitError() << "Returned value '" << retVal << "' is an i32 and was not entangled "
                                         << "by netqasm.eprs operation.";
                }
                continue;
            }
        }
        returnOp.emitError() << "Returned value '" << retVal << "' is not an i1 or i32.";
    }

    return success();
}

LogicalResult netqasm::LocalRoutineOp::validateNestedInstructions() {
    // EPRS-related operations are not allowed in local routines
    if (containsEPRSOperation(this->getOperation())) {
        this->emitError("EPRS-related operations are not allowed in local routines.");
        return failure();
    }
    return success();
}

std::string netqasm::NetQASMDialect::getAllowedDialectNames() {
    return getDialectNamesList<
#define GET_ALLOWED_DIALECTS
#include "Dialect/NetQASM/NetQASM.h"
            >();
}
