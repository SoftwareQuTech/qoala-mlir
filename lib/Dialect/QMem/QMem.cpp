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
    auto buildFuncType = [](Builder &builder, ArrayRef<Type> argTypes, ArrayRef<Type> results,
                            function_interface_impl::VariadicFlag,
                            std::string &) { return builder.getFunctionType(argTypes, results); };

    return function_interface_impl::parseFunctionOp(parser, result, /*allowVariadic=*/false,
                                                    getFunctionTypeAttrName(result.name), buildFuncType,
                                                    getArgAttrsAttrName(result.name), getResAttrsAttrName(result.name));
}

void FuncOp::build(OpBuilder &builder, OperationState &state, StringRef name, FunctionType type,
                   ArrayRef<NamedAttribute> attrs, ArrayRef<DictionaryAttr> argAttrs) {
    state.addAttribute(mlir::SymbolTable::getSymbolAttrName(), builder.getStringAttr(name));
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

void FuncOp::print(OpAsmPrinter &p) {
    function_interface_impl::printFunctionOp(p, *this, /*isVariadic=*/false, getFunctionTypeAttrName(),
                                             getArgAttrsAttrName(), getResAttrsAttrName());
}

Operation *CnotOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<CnotOp>(loc, getQin0(), getQin1());
}

std::vector<Operation *> CnotOp::getOpsAllocatingUsedQubits() {
    return {this->getQin0().getDefiningOp(), this->getQin1().getDefiningOp()};
}

Operation *CrotXOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<CrotXOp>(loc, getQin0(), getQin1(), getAngle());
}

std::vector<Operation *> CrotXOp::getOpsAllocatingUsedQubits() {
    return {this->getQin0().getDefiningOp(), this->getQin1().getDefiningOp()};
}

Operation *CzOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<CzOp>(loc, getQin0(), getQin1());
}

std::vector<Operation *> CzOp::getOpsAllocatingUsedQubits() {
    return {this->getQin0().getDefiningOp(), this->getQin1().getDefiningOp()};
}

Operation *EprsMeasureOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<EprsMeasureOp>(loc, getOutcome().getType(), getQ(), getRemoteAttr());
}

std::vector<Operation *> EprsMeasureOp::getOpsAllocatingUsedQubits() { return {this->getQ().getDefiningOp()}; }

Operation *EprsOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<EprsOp>(loc, getQ(), getRemoteAttr());
}

std::vector<Operation *> EprsOp::getOpsAllocatingUsedQubits() { return {this->getQ().getDefiningOp()}; }

Operation *FuncOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<FuncOp>(loc, getSymNameAttr(), getFunctionTypeAttr(), getSymVisibilityAttr(),
                                     getArgAttrsAttr(), getResAttrsAttr());
}

std::vector<Operation *> FuncOp::getOpsAllocatingUsedQubits() { return {}; }

Operation *HadamardOp::simpleClone(OpBuilder &builder, const Location loc) { return builder.create<HadamardOp>(loc, getQ()); }

std::vector<Operation *> HadamardOp::getOpsAllocatingUsedQubits() { return {this->getQ().getDefiningOp()}; }

Operation *InitOp::simpleClone(OpBuilder &builder, const Location loc) { return builder.create<InitOp>(loc, getQ()); }

std::vector<Operation *> InitOp::getOpsAllocatingUsedQubits() { return {this->getQ().getDefiningOp()}; }

Operation *MeasureOp::simpleClone(OpBuilder &builder, const Location loc) { return builder.create<MeasureOp>(loc, getQ()); }

std::vector<Operation *> MeasureOp::getOpsAllocatingUsedQubits() { return {this->getQ().getDefiningOp()}; }

Operation *QAllocOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<QAllocOp>(loc, getQ().getType());
}

std::vector<Operation *> QAllocOp::getOpsAllocatingUsedQubits() { return {this->getQ().getDefiningOp()}; }

Operation *RecvFloatsOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<RecvFloatsOp>(loc, getCout().getType(), getRemoteAttr(), getLengthAttr());
}

std::vector<Operation *> RecvFloatsOp::getOpsAllocatingUsedQubits() { return {}; }

Operation *RecvIntsOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<RecvIntsOp>(loc, getCout().getType(), getRemoteAttr(), getLengthAttr());
}

std::vector<Operation *> RecvIntsOp::getOpsAllocatingUsedQubits() { return {}; }

Operation *RemoteOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<RemoteOp>(loc, getSymNameAttr(), getSymVisibilityAttr());
}

std::vector<Operation *> RemoteOp::getOpsAllocatingUsedQubits() { return {}; }

Operation *ReturnOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<ReturnOp>(loc, getOperands());
}

std::vector<Operation *> ReturnOp::getOpsAllocatingUsedQubits() { return {}; }

Operation *RotateXOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<RotateXOp>(loc, getQ(), getAngle());
}

std::vector<Operation *> RotateXOp::getOpsAllocatingUsedQubits() { return {this->getQ().getDefiningOp()}; }

Operation *RotateYOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<RotateYOp>(loc, getQ(), getAngle());
}

std::vector<Operation *> RotateYOp::getOpsAllocatingUsedQubits() { return {this->getQ().getDefiningOp()}; }

Operation *RotateZOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<RotateZOp>(loc, getQ(), getAngle());
}

std::vector<Operation *> RotateZOp::getOpsAllocatingUsedQubits() { return {this->getQ().getDefiningOp()}; }

Operation *SendFloatsOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<SendFloatsOp>(loc, getCin(), getRemoteAttr());
}

std::vector<Operation *> SendFloatsOp::getOpsAllocatingUsedQubits() { return {}; }

Operation *SendIntsOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<SendIntsOp>(loc, getCin(), getRemoteAttr());
}

std::vector<Operation *> SendIntsOp::getOpsAllocatingUsedQubits() { return {}; }

Operation *CrotXIntOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<CrotXIntOp>(loc, getQin0(), getQin1(), getNValAttr(), getExpValAttr());
}

std::vector<Operation *> CrotXIntOp::getOpsAllocatingUsedQubits() {
    return {this->getQin0().getDefiningOp(), this->getQin1().getDefiningOp()};
}

Operation *RotateXIntOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<RotateXIntOp>(loc, getQ(), getNValAttr(), getExpValAttr());
}

std::vector<Operation *> RotateXIntOp::getOpsAllocatingUsedQubits() { return {this->getQ().getDefiningOp()}; }

Operation *RotateYIntOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<RotateYIntOp>(loc, getQ(), getNValAttr(), getExpValAttr());
}

std::vector<Operation *> RotateYIntOp::getOpsAllocatingUsedQubits() { return {this->getQ().getDefiningOp()}; }

Operation *RotateZIntOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<RotateZIntOp>(loc, getQ(), getNValAttr(), getExpValAttr());
}

Operation *SendFloatOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<SendFloatOp>(loc, getCin(), getRemoteAttr());
}

std::vector<Operation *> SendFloatOp::getOpsAllocatingUsedQubits() { return {}; }

Operation *SendIntOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<SendIntOp>(loc, getCin(), getRemoteAttr());
}

std::vector<Operation *> SendIntOp::getOpsAllocatingUsedQubits() { return {}; }

Operation *RecvFloatOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<RecvFloatOp>(loc, getCout().getType(), getRemoteAttr(), getLengthAttr());
}

std::vector<Operation *> RecvFloatOp::getOpsAllocatingUsedQubits() { return {}; }

Operation *RecvIntOp::simpleClone(OpBuilder &builder, const Location loc) {
    return builder.create<RecvIntsOp>(loc, getCout().getType(), getRemoteAttr(), getLengthAttr());
}

std::vector<Operation *> RecvIntOp::getOpsAllocatingUsedQubits() { return {}; }

std::vector<Operation *> RotateZIntOp::getOpsAllocatingUsedQubits() { return {this->getQ().getDefiningOp()}; }

Value InitOp::getDefiningQubit() { return this->getQ(); }

Value EprsOp::getDefiningQubit() { return this->getQ(); }

Value EprsMeasureOp::getDefiningQubit() { return this->getQ(); }
