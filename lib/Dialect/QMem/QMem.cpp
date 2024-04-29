#include "Dialect/QMem/QMem.h"
#include "Dialect/QMem/QMemDialect.h"
#include "mlir/Interfaces/FunctionImplementation.h"

using namespace mlir;
using namespace qoala::dialects::qmem;

// include generated source code for operations
#define GET_OP_CLASSES
#include "Dialect/QMem/QMem.cpp.inc"

// include generated source code for types
#define GET_TYPEDEF_CLASSES
#include "Dialect/QMem/QMemTypes.cpp.inc"

// include generated "dispatcher" of the operation interface
#include "Analysis/Helpers/SimpleCloneInterface.cpp.inc"


/* Parse and print functions "ported" from func.func: parse and print */
ParseResult FuncOp::parse(OpAsmParser &parser, OperationState &result) {
    auto buildFuncType =
            [](Builder &builder, ArrayRef<Type> argTypes, ArrayRef<Type> results,
               function_interface_impl::VariadicFlag,
               std::string &) { return builder.getFunctionType(argTypes, results); };

    return function_interface_impl::parseFunctionOp(
            parser, result, /*allowVariadic=*/false,
            getFunctionTypeAttrName(result.name), buildFuncType,
            getArgAttrsAttrName(result.name), getResAttrsAttrName(result.name));
}


void FuncOp::build(OpBuilder &builder, OperationState &state, StringRef name,
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

void FuncOp::print(OpAsmPrinter &p) {
    function_interface_impl::printFunctionOp(
            p, *this, /*isVariadic=*/false, getFunctionTypeAttrName(),
            getArgAttrsAttrName(), getResAttrsAttrName());
}

Operation *CnotOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<CnotOp>(loc, getQin0(), getQin1());
}

Operation *CrotXOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<CrotXOp>(loc, getQin0(), getQin1(), getAngle());
}

Operation *CzOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<CzOp>(loc, getQin0(), getQin1());
}

Operation *EprsMeasureOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<EprsMeasureOp>(loc, getOutcome().getType(), getQ(), getRemoteAttr());
}

Operation *EprsOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<EprsOp>(loc, getQ(), getRemoteAttr());
}

Operation *FuncOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<FuncOp>(loc, getSymNameAttr(),
                                  getFunctionTypeAttr(), getSymVisibilityAttr(),
                                  getArgAttrsAttr(), getResAttrsAttr());
}

Operation *HadamardOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<HadamardOp>(loc, getQ());
}

Operation *InitOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<InitOp>(loc, getQ());
}

Operation *MeasureOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<MeasureOp>(loc, getQ());
}

Operation *QAllocOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<QAllocOp>(loc, getQ().getType());
}

Operation *RecvFloatsOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<RecvFloatsOp>(loc, getCout().getType(), getRemoteAttr(), getLengthAttr());
}

Operation *RecvIntsOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<RecvIntsOp>(loc, getCout().getType(), getRemoteAttr(), getLengthAttr());
}

Operation *RemoteOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<RemoteOp>(loc, getSymNameAttr(), getSymVisibilityAttr());
}

Operation *ReturnOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<ReturnOp>(loc, getOperands());
}

Operation *RotateXOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<RotateXOp>(loc, getQ(), getAngle());
}

Operation *RotateYOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<RotateYOp>(loc, getQ(), getAngle());
}

Operation *RotateZOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<RotateZOp>(loc, getQ(), getAngle());
}

Operation *SendFloatsOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<SendFloatsOp>(loc, getCin(), getRemoteAttr());
}

Operation *SendIntsOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<SendIntsOp>(loc, getCin(), getRemoteAttr());
}
