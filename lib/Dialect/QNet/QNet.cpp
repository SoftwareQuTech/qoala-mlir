#include "mlir/Interfaces/FunctionImplementation.h"

#include "Dialect/QNet/QNet.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/Builders.h"

using namespace mlir;
using namespace qoala::dialects::qnet;
using namespace qoala::helpers;

// include generated source code for operations
#define GET_OP_CLASSES
#include "Dialect/QNet/QNet.cpp.inc"

// include generated "dispatcher" of the operation interface
#include "Analysis/Helpers/QNetInterfaces.cpp.inc"

/* Parse and print functions "ported" from func.func: parse and print */
ParseResult FuncOp::parse(OpAsmParser &parser, OperationState &result) {
    auto buildFuncType = [](Builder &builder, ArrayRef<Type> argTypes, ArrayRef<Type> results,
                            function_interface_impl::VariadicFlag,
                            std::string &) { return builder.getFunctionType(argTypes, results); };

    return function_interface_impl::parseFunctionOp(parser, result, /*allowVariadic=*/false,
                                                    getFunctionTypeAttrName(result.name), buildFuncType,
                                                    getArgAttrsAttrName(result.name), getResAttrsAttrName(result.name));
}

void FuncOp::print(OpAsmPrinter &p) {
    function_interface_impl::printFunctionOp(p, *this, /*isVariadic=*/false, getFunctionTypeAttrName(),
                                             getArgAttrsAttrName(), getResAttrsAttrName());
}

static void replaceAngleOperandWithConst(Operation *op, const uint32_t angleIdx, FloatAttr fa) {
    OpBuilder b(op);
    b.setInsertionPoint(op);
    auto c = b.create<arith::ConstantOp>(op->getLoc(), fa);
    op->setOperand(angleIdx, c.getResult());
}

RotationAxis RotXOp::getAxis() { return RotationAxis::X; }

void RotXOp::setAngleAttr(Attribute angle) {
    const auto fa = dyn_cast_or_null<FloatAttr>(angle);
    assert(fa && "setAngleAttr expects FloatAttr");
    // operands: (qin, angle)
    replaceAngleOperandWithConst(getOperation(), /*angleIdx=*/1, fa);
}

RotationAxis RotYOp::getAxis() { return RotationAxis::Y; }

void RotYOp::setAngleAttr(Attribute angle) {
    const auto fa = dyn_cast_or_null<FloatAttr>(angle);
    assert(fa && "setAngleAttr expects FloatAttr");
    replaceAngleOperandWithConst(getOperation(), /*angleIdx=*/1, fa);
}

RotationAxis RotZOp::getAxis() { return RotationAxis::Z; }

void RotZOp::setAngleAttr(Attribute angle) {
    const auto fa = dyn_cast_or_null<FloatAttr>(angle);
    assert(fa && "setAngleAttr expects FloatAttr");
    replaceAngleOperandWithConst(getOperation(), /*angleIdx=*/1, fa);
}

RotationAxis CrotXOp::getAxis() { return RotationAxis::X; }

void CrotXOp::setAngleAttr(Attribute angle) {
    const auto fa = dyn_cast_or_null<FloatAttr>(angle);
    assert(fa && "setAngleAttr expects FloatAttr");
    replaceAngleOperandWithConst(getOperation(), /*angleIdx=*/2, fa);
}
