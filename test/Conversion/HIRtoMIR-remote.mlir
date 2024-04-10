// RUN: qoala-opt %s --convert-qnet-to-qmem | FileCheck %s

// CHECK: module

module {
  // CHECK: qmem.func @test_remote_quantum_program()
  qnet.func @test_remote_quantum_program() {
    // CHECK: qmem.remote @Bob
    qnet.remote @Bob

    // CHECK: %[[RCVINTS:.*]] = qmem.recv_ints  {remote = @Bob} : tensor<2xi32>
    %received_ints = qnet.recv_ints {remote = @Bob} : tensor<2xi32>

    %c0 = arith.constant 0 : i32
    %c5 = arith.constant 5 : i32
    %ints_to_send = tensor.from_elements %c0, %c5 : tensor<2xi32>

    // CHECK : qmem.send_ints %ints_to_send {remote = @Bob}
    qnet.send_ints %ints_to_send {remote = @Bob} : tensor<2xi32>

    // CHECK: %[[RCVFLOATS:.*]] = qmem.recv_floats {remote = @Bob} : tensor<2xf32>
    %received_floats = qnet.recv_floats {remote = @Bob} : tensor<2xf32>

    %cf0 = arith.constant 1.000000e+00 : f32
    %cf5 = arith.constant 5.000000e+00 : f32
    %floats_to_send = tensor.from_elements %cf0, %cf5 : tensor<2xf32>

    // CHECK : qmem.send_floats %floats_to_send {remote = @Bob}
    qnet.send_floats %floats_to_send {remote = @Bob} : tensor<2xf32>

    // CHECK: qmem.return
    qnet.return
  }
}