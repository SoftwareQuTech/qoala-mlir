// RUN: qoala-opt %s --lower-qoala-mir-to-lir=disable-unfold-comm-ops | FileCheck %s

// There is NO quantum instructions here, so we don't expect any "functionization" effect

// CHECK: module
module {
  // CHECK: qremote.remote @[[REMOTEBOB:.*]]
  qmem.remote @Bob

  // CHECK: qoalahost.main_func @test_remote_quantum_program()
  qmem.func @test_remote_quantum_program() {
    // tensor.from elements are folded into a "arith.constant dense", and placed at the start of the file
    // CHECK: %[[FROM_ELEM:.*]] = arith.constant dense<[0, 5]> : tensor<2xi32>
    // CHECK-NEXT: %[[FROM_ELEM_1:.*]] = arith.constant dense<[1.{{0*}}e+00, 5.{{0*}}e+00]> : tensor<2xf32>
    // CHECK-NEXT: qoalahost.nop_term

    // CHECK: ^[[BLOCK_1:.*]]:
    // CHECK: qoalahost.recv_ints {length = 2 : i32, remote = @[[REMOTEBOB]]} : tensor<2xi32>
    %rec_int_0 = qmem.recv_int {remote = @Bob} : i32
    %rec_int_1 = qmem.recv_int {remote = @Bob} : i32

    // CHECK: ^[[BLOCK_2:.*]]:
    %c0_i32 = arith.constant 0 : i32
    %c5_i32 = arith.constant 5 : i32
    %prep_int = arith.addi %c5_i32, %rec_int_1 : i32

    // CHECK: qoalahost.send_int %[[FROM_ELEM]] {remote = @[[REMOTEBOB]]} : tensor<2xi32>
    qmem.send_int %c0_i32 {remote = @Bob} : i32
    qmem.send_int %prep_int {remote = @Bob} : i32
    // CHECK-NEXT: qoalahost.nop_term

    // CHECK: ^[[BLOCK_3:.*]]:
    // CHECK: qoalahost.recv_floats {length = 2 : i32, remote = @[[REMOTEBOB]]} : tensor<2xf32>
    %rec_float_0= qmem.recv_float {remote = @Bob} : f32
    %rec_float_1= qmem.recv_float {remote = @Bob} : f32

    // CHECK: ^[[BLOCK_4:.*]]:
    %cst = arith.constant 1.000000e+00 : f32
    %cst_0 = arith.constant 5.000000e+00 : f32

    %prep_float = arith.addf %cst_0, %rec_float_0 : f32

    // CHECK: qoalahost.send_floats %[[FROM_ELEM_1]] {remote = @[[REMOTEBOB]]} : tensor<2xf32>
    qmem.send_float %cst_0 {remote = @Bob} : f32
    qmem.send_float %prep_float {remote = @Bob} : f32
    // CHECK: qoalahost.return
    qmem.return
  }
}