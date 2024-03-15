module {
  func.func @simple_arith_program_immediates() {
    %c10_i32 = arith.constant 10 : i32
    %c20_i32 = arith.constant 20 : i32
    %0 = arith.addi %c10_i32, %c20_i32 : i32
    %cst = arith.constant 1.000000e+01 : f32
    %cst_0 = arith.constant 2.000000e+01 : f32
    %1 = arith.subf %cst, %cst_0 : f32
    %c5_i32 = arith.constant 5 : i32
    %2 = arith.muli %c5_i32, %0 : i32
    %c5_i32_1 = arith.constant 5 : i32
    %3 = arith.divui %2, %c5_i32_1 : i32
    %cst_2 = arith.constant 2.000000e+01 : f32
    %4 = arith.mulf %1, %cst_2 : f32
    %cst_3 = arith.constant 4.000000e+00 : f32
    %5 = arith.divf %4, %cst_3 : f32
    return
  }
}
