// UNSUPPORTED: true
// This program is only used for the qoala-translate tool: it's not supposed to be a full test case
// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @__qoala_wrapper0() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.local_routine @__qoala_wrapper1() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.local_routine @__qoala_wrapper2(%arg0: i32, %arg1: i32, %arg2: i32, %arg3: i32, %arg4: i32, %arg5: i32, %arg6: i32, %arg7: i32, %arg8: i32, %arg9: i32) -> (i1, i1) {
    netqasm.rot_x %arg0, %arg1, %arg2
    netqasm.rot_y %arg0, %arg3, %arg4
    netqasm.rot_z %arg0, %arg5, %arg6
    netqasm.hadamard %arg7
    netqasm.cnot %arg0, %arg7
    netqasm.cz %arg0, %arg7
    netqasm.crot_x %arg0, %arg7, %arg8, %arg9
    %0 = netqasm.measure %arg0 : i1
    %1 = netqasm.measure %arg7 : i1
    netqasm.return %0, %1 : i1, i1
  }
  qoalahost.main_func @test_local_quantum_program() {
    %0 = qoalahost.call @__qoala_wrapper0() : () -> i32
    %1 = qoalahost.call @__qoala_wrapper1() : () -> i32
    %cst = arith.constant 0.0306796152 : f32
    %2:2 = qoalahost.call @__qoala_convert_float_angle(%cst) : (f32) -> (i32, i32)
    %cst_0 = arith.constant 1.050000e+01 : f32
    %3:2 = qoalahost.call @__qoala_convert_float_angle(%cst_0) : (f32) -> (i32, i32)
    %cst_1 = arith.constant 2.120000e+01 : f32
    %4:2 = qoalahost.call @__qoala_convert_float_angle(%cst_1) : (f32) -> (i32, i32)
    %cst_2 = arith.constant 2.710000e+00 : f32
    %5:2 = qoalahost.call @__qoala_convert_float_angle(%cst_2) : (f32) -> (i32, i32)
    %6:2 = qoalahost.call @__qoala_wrapper2(%0, %2#0, %2#1, %3#0, %3#1, %4#0, %4#1, %1, %5#0, %5#1) : (i32, i32, i32, i32, i32, i32, i32, i32, i32, i32) -> (i1, i1)
    qoalahost.return
  }
}

