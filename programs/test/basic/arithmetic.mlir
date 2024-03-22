module {
  func.func @simple_arith_program() {
    %c10_i32 = arith.constant 10 : i32
    %c20_i32 = arith.constant 20 : i32
    %0 = arith.addi %c10_i32, %c20_i32 : i32
    %1 = arith.subi %c10_i32, %c20_i32 : i32
    %2 = arith.muli %c10_i32, %c20_i32 : i32
    %3 = arith.divui %c10_i32, %c20_i32 : i32
    %cst = arith.constant 1.000000e+01 : f32
    %cst_0 = arith.constant 2.000000e+01 : f32
    %4 = arith.addf %cst, %cst_0 : f32
    %5 = arith.subf %cst, %cst_0 : f32
    %6 = arith.mulf %cst, %cst_0 : f32
    %7 = arith.divf %cst, %cst_0 : f32
    return
  }
}
