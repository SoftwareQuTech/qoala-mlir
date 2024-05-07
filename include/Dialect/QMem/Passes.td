#ifndef QMEM_PASSES
#define QMEM_PASSES

include "mlir/Pass/PassBase.td"

def QMemSimpleFunctionize : Pass<"qmem-simple-functionize", "mlir::ModuleOp"> {
    let summary = "PoC; DO NOT USE - Extracts each QMem quantum operation into a single func.func function";
    let constructor = "mlir::createQMemSimpleFunctionize()";
    let dependentDialects = ["::qoala::dialects::qmem::QMemDialect", "::mlir::func::FuncDialect"];
}

def QMemConvertIntegerToFloatRotations : Pass<"qmem-lower-float-rotations", "mlir::ModuleOp"> {
    let summary = "Converts Rotations (X, Y, Z and CrotX) that use f32 angle arguments into their counterparts using i32 arguments.";
    let constructor = "mlir::createQMemF32RotationsConversion()";
    let dependentDialects = ["::qoala::dialects::qmem::QMemDialect"];
}

#endif // QMEM_PASSES