// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// CHECK: module
module {
  // CHECK: qremote.remote @[[REMOTEBOB:.*]]
  qmem.remote @Bob

  // CHECK: qoalahost.main_func @test_remote_quantum_program()
  qmem.func @test_remote_quantum_program() {
    // CHECK: qoalahost.recv_ints {length = 2 : i32, remote = @[[REMOTEBOB]]} : tensor<2xi32>
    %0 = qmem.recv_ints {length = 2 : i32, remote = @Bob} : tensor<2xi32>

    %c0_i32 = arith.constant 0 : i32
    %c5_i32 = arith.constant 5 : i32

    // CHECK: %[[FROM_ELEM:.*]] = tensor.from_elements
    %from_elements = tensor.from_elements %c0_i32, %c5_i32 : tensor<2xi32>

    // CHECK: qoalahost.send_ints %[[FROM_ELEM]] {remote = @[[REMOTEBOB]]} : tensor<2xi32>
    qmem.send_ints %from_elements {remote = @Bob} : tensor<2xi32>

    // CHECK: qoalahost.recv_floats {length = 2 : i32, remote = @[[REMOTEBOB]]} : tensor<2xf32>
    %1 = qmem.recv_floats {length = 2 : i32, remote = @Bob} : tensor<2xf32>

    %cst = arith.constant 1.000000e+00 : f32
    %cst_0 = arith.constant 5.000000e+00 : f32

    // CHECK: %[[FROM_ELEM_1:.*]] = tensor.from_elements
    %from_elements_1 = tensor.from_elements %cst, %cst_0 : tensor<2xf32>

    // CHECK: qoalahost.send_floats %[[FROM_ELEM_1]] {remote = @[[REMOTEBOB]]} : tensor<2xf32>
    qmem.send_floats %from_elements_1 {remote = @Bob} : tensor<2xf32>
    // CHECK: qoalahost.return
    qmem.return
  }
}