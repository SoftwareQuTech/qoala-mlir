// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// CHECK: module
module {
  // CHECK: qremote.remote @[[REMOTEBOB:.*]]
  qmem.remote @Bob

  // CHECK: qoalahost.main_func @test_remote_quantum_program()
  qmem.func @test_send_values() {
    // tensor.from elements are folded into a "arith.constant dense", and placed at the start of the file
    // CHECK: %[[FROM_ELEM:.*]] = arith.constant dense<[0, 5]> : tensor<2xi32>
    // CHECK-NEXT: %[[FROM_ELEM_1:.*]] = arith.constant dense<[1.{{0*}}e+00, 5.{{0*}}e+00]> : tensor<2xf32>
    // CHECK-NEXT: qoalahost.nop_term

    %c0_i32 = arith.constant 0 : i32
    %c5_i32 = arith.constant 5 : i32

    %from_elements = tensor.from_elements %c0_i32, %c5_i32 : tensor<2xi32>

    // CHECK: ^[[BLOCK_1:.*]]:
    // CHECK: qoalahost.send_int %[[FROM_ELEM]] {remote = @[[REMOTEBOB]]} i32
    // CHECK: qoalahost.send_int %[[FROM_ELEM]] {remote = @[[REMOTEBOB]]} i32
    qmem.send_ints %from_elements {remote = @Bob} : tensor<2xi32>
    // CHECK-NEXT: qoalahost.nop_t

    %cst = arith.constant 1.000000e+00 : f32
    %cst_0 = arith.constant 5.000000e+00 : f32

    %from_elements_1 = tensor.from_elements %cst, %cst_0 : tensor<2xf32>

    // CHECK: ^[[BLOCK_2:.*]]:
    // CHECK: qoalahost.send_float %[[FROM_ELEM_1]] {remote = @[[REMOTEBOB]]} : f32
    // CHECK: qoalahost.send_float %[[FROM_ELEM_1]] {remote = @[[REMOTEBOB]]} : f32
    qmem.send_floats %from_elements_1 {remote = @Bob} : tensor<2xf32>
    // CHECK: qoalahost.return
    qmem.return
  }
}