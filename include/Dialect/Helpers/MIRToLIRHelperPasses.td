#ifndef QMEM_PASSES
#define QMEM_PASSES

include "mlir/Pass/PassBase.td"

def SimpleFunctionize : Pass<"simple-functionize", "mlir::ModuleOp"> {
    let summary = "PoC; DO NOT USE - Extracts each QMem quantum operation into a single func.func function";
    let dependentDialects = ["qoala::dialects::qmem::QMemDialect", "mlir::func::FuncDialect"];
}

def ConvertIntegerToFloatRotations : Pass<"lower-float-rotations", "qoala::dialects::qmem::FuncOp"> {
    let summary = "Converts Rotations (X, Y, Z and CrotX) that use f32 angle arguments into their counterparts using i32 arguments.";
    let dependentDialects = ["qoala::dialects::qmem::QMemDialect"];
}

//===----------------------------------------------------------------------===//
// Helper pass for functionizing quantum operations
//===----------------------------------------------------------------------===//

def FunctionizeQuantumOps : Pass<"functionize", "mlir::ModuleOp"> {
    let summary = "Functionizes the quantum operations inside the main function of the module";
    let options = [
        Option<"useSimpleFunctionize", "use-simple-functionize", "bool", /*default=*/"false",
               "WARNING: Use simple functionize; use a single quantum operation in a function wrapper.">,
        Option<"maxOpsPerGroup", "max-ops-per-group", "uint32_t", /*default=*/"0",
               "Set the max number of operations to place per group. This option only affects the per-qubit grouping functionization">
    ];
    let dependentDialects = ["qoala::dialects::qmem::QMemDialect", "mlir::func::FuncDialect"];
}

//===----------------------------------------------------------------------===//
// Helper pass to fold constants
//===----------------------------------------------------------------------===//

def FoldConstants : Pass<"fold-constants"> {
    let summary = "Folds constants in the nested operations";
    let dependentDialects = [];
}

//===----------------------------------------------------------------------===//
// Helper pass to insert builtin angle conversion declartion function
//===----------------------------------------------------------------------===//

def AngleConversionDeclaration : Pass<"insert-convert-float-angle", "mlir::ModuleOp"> {
    let summary = "Inserts the '__qoala_convert_float_angle' declaration in the module.";
    let dependentDialects = [];
}


#endif // QMEM_PASSES