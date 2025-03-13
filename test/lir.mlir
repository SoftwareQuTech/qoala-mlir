// UNSUPPORTED: true
// This program is only used for the qoala-translate tool: it's not supposed to be a full test case
// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @__qoala_wrapper0() -> i32 {
    %cst = arith.constant 25 : i32
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.local_routine @__qoala_wrapper1() -> i32 {
    %cst = arith.constant 15 : i32
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.local_routine @__qoala_wrapper2(%arg0: i32, %arg1: i32) -> (i1, i1) {
    netqasm.rot_x %arg0 (1 : ui32, 2 : ui32)
    netqasm.rot_y %arg0 (1 : ui32, 1 : ui32)
    netqasm.rot_z %arg0 (3 : ui32, 4 : ui32)
    netqasm.hadamard %arg1
    netqasm.cnot %arg0, %arg1
    netqasm.cz %arg0, %arg1
    netqasm.crot_x %arg0, %arg1 (1 : ui32, 0 : ui32)
    %0 = netqasm.measure %arg0 : i1
    %1 = netqasm.measure %arg1 : i1
    netqasm.return %0, %1 : i1, i1
  }
  qoalahost.main_func @test_local_quantum_program() {
    %0 = qoalahost.call @__qoala_wrapper0() : () -> i32
    %1 = qoalahost.call @__qoala_wrapper1() : () -> i32
    %cst = arith.constant 3 : i32
    %2:2 = qoalahost.call @__qoala_wrapper2(%0, %1) : (i32, i32) -> (i1, i1)
    qoalahost.return
  }
}

