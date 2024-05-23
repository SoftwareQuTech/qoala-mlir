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
#include "Analysis/Helpers/QMemInterfaces.cpp.inc"


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

Value CnotOp::getOperationQubit() {
    return this->getQin0();
}

Operation *CrotXOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<CrotXOp>(loc, getQin0(), getQin1(), getAngle());
}

Value CrotXOp::getOperationQubit() {
    return this->getQin0();
}

Operation *CzOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<CzOp>(loc, getQin0(), getQin1());
}

Value CzOp::getOperationQubit() {
    return this->getQin0();
}

Operation *EprsMeasureOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<EprsMeasureOp>(loc, getOutcome().getType(), getQ(), getRemoteAttr());
}

Value EprsMeasureOp::getOperationQubit() {
    return this->getQ();
}

Operation *EprsOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<EprsOp>(loc, getQ(), getRemoteAttr());
}

Value EprsOp::getOperationQubit() {
    return this->getQ();
}

Operation *FuncOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<FuncOp>(loc, getSymNameAttr(),
                                  getFunctionTypeAttr(), getSymVisibilityAttr(),
                                  getArgAttrsAttr(), getResAttrsAttr());
}

Value FuncOp::getOperationQubit() {
    return nullptr;
}

Operation *HadamardOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<HadamardOp>(loc, getQ());
}

Value HadamardOp::getOperationQubit() {
    return this->getQ();
}

Operation *InitOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<InitOp>(loc, getQ());
}

Value InitOp::getOperationQubit() {
    return this->getQ();
}

Operation *MeasureOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<MeasureOp>(loc, getQ());
}

Value MeasureOp::getOperationQubit() {
    return this->getQ();
}

Operation *QAllocOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<QAllocOp>(loc, getQ().getType());
}

Value QAllocOp::getOperationQubit() {
    return this->getQ();
}

Operation *RecvFloatsOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<RecvFloatsOp>(loc, getCout().getType(), getRemoteAttr(), getLengthAttr());
}

Value RecvFloatsOp::getOperationQubit() {
    return nullptr;
}

Operation *RecvIntsOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<RecvIntsOp>(loc, getCout().getType(), getRemoteAttr(), getLengthAttr());
}

Value RecvIntsOp::getOperationQubit() {
    return nullptr;
}

Operation *RemoteOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<RemoteOp>(loc, getSymNameAttr(), getSymVisibilityAttr());
}

Value RemoteOp::getOperationQubit() {
    return nullptr;
}

Operation *ReturnOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<ReturnOp>(loc, getOperands());
}

Value ReturnOp::getOperationQubit() {
    return nullptr;
}

Operation *RotateXOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<RotateXOp>(loc, getQ(), getAngle());
}

Value RotateXOp::getOperationQubit() {
    return this->getQ();
}

Operation *RotateYOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<RotateYOp>(loc, getQ(), getAngle());
}

Value RotateYOp::getOperationQubit() {
    return this->getQ();
}

Operation *RotateZOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<RotateZOp>(loc, getQ(), getAngle());
}

Value RotateZOp::getOperationQubit() {
    return this->getQ();
}

Operation *SendFloatsOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<SendFloatsOp>(loc, getCin(), getRemoteAttr());
}

Value SendFloatsOp::getOperationQubit() {
    return nullptr;
}

Operation *SendIntsOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<SendIntsOp>(loc, getCin(), getRemoteAttr());
}

Value SendIntsOp::getOperationQubit() {
    return nullptr;
}

Operation *CrotXIntOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<CrotXIntOp>(loc, getQin0(), getQin1(), getAngleNum(), getAngleDenom());
}

Value CrotXIntOp::getOperationQubit() {
    return this->getQin0();
}

Operation *RotateXIntOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<RotateXIntOp>(loc, getQ(), getAngleNum(), getAngleDenom());
}

Value RotateXIntOp::getOperationQubit() {
    return this->getQ();
}

Operation *RotateYIntOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<RotateYIntOp>(loc, getQ(), getAngleNum(), getAngleDenom());
}

Value RotateYIntOp::getOperationQubit() {
    return this->getQ();
}

Operation *RotateZIntOp::simpleClone(OpBuilder &builder, Location loc) {
    return builder.create<RotateZIntOp>(loc, getQ(), getAngleNum(), getAngleDenom());
}

Value RotateZIntOp::getOperationQubit() {
    return this->getQ();
}
