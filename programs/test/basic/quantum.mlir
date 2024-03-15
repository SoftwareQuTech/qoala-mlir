module {
  func.func @complex_quantum_program() {
    %c20_i32 = arith.constant 20 : i32
    %c30_i32 = arith.constant 30 : i32
    %cst = arith.constant 2.120000e+01 : f32
    %0 = "hir.new_qubit"() : () -> !hir.qubit
    %1 = "hir.new_qubit"() : () -> !hir.qubit
    %c10_i32 = arith.constant 10 : i32
    %c30_i32_0 = arith.constant 30 : i32
    %cst_1 = arith.constant 0.000000e+00 : f32
    %2 = "hir.rot_x"(%0, %cst_1) : (!hir.qubit, f32) -> !hir.qubit
    %c10_i32_2 = arith.constant 10 : i32
    %cst_3 = arith.constant 1.050000e+01 : f32
    %3 = "hir.rot_y"(%0, %cst_3) : (!hir.qubit, f32) -> !hir.qubit
    %4 = "hir.rot_z"(%0, %cst) : (!hir.qubit, f32) -> !hir.qubit
    %qout0, %qout1 = "hir.cnot"(%0, %1) : (!hir.qubit, !hir.qubit) -> (!hir.qubit, !hir.qubit)
    %5 = "hir.measure"(%0) : (!hir.qubit) -> i1
    return
  }
}
