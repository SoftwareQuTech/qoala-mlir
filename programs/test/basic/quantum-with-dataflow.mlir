module {
  func.func @complex_quantum_program() {
    %c20_i32 = arith.constant 20 : i32
    %c30_i32 = arith.constant 30 : i32
    %cst = arith.constant 2.120000e+01 : f32
    %0 = hir.new_qubit : !hir.qubit
    %1 = hir.new_qubit : !hir.qubit
    %2 = hir.measure %0 : i1
    return
  }
}
