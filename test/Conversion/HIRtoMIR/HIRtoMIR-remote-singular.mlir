// RUN: qoala-opt %s --lower-qoala-hir-to-mir | FileCheck %s

// CHECK: module

module {
  // CHECK: qmem.remote @[[BOBREMOTE:.*]]
  qnet.remote @Bob
  // CHECK: qmem.func @test_remote_quantum_program_singular()
  qnet.func @test_remote_quantum_program_singular() {

    // CHECK: %[[RCVINT_A:.*]] = qmem.recv_int  {remote = @[[BOBREMOTE]]} : i32
    %received_int_a = qnet.recv_int {remote = @Bob} : i32
    // CHECK: %[[RCVINT_A:.*]] = qmem.recv_int  {remote = @[[BOBREMOTE]]} : i32
    %received_int_b = qnet.recv_int {remote = @Bob} : i32

    // CHECK: %[[CST_INT_A:.*]] = arith.constant
    %c0 = arith.constant 0 : i32
    // CHECK: %[[CST_INT_B:.*]] = arith.constant
    %c5 = arith.constant 5 : i32

    // CHECK : qmem.send_int %[[CST_INT_A]] {remote = @[[BOBREMOTE]]}
    qnet.send_int %c0 {remote = @Bob} : i32
    // CHECK : qmem.send_int %[[CST_INT_B]] {remote = @[[BOBREMOTE]]}
    qnet.send_int %c5 {remote = @Bob} : i32

    // CHECK: %[[RCVFLOAT_A:.*]] = qmem.recv_float {remote = @[[BOBREMOTE]]} : f32
    %received_float_a = qnet.recv_float {remote = @Bob} : f32
    // CHECK: %[[RCVFLOAT_B:.*]] = qmem.recv_float {remote = @[[BOBREMOTE]]} : f32
    %received_float_b = qnet.recv_float {remote = @Bob} : f32

    // CHECK: %[[CST_FLT_A:.*]] = arith.constant
    %cf0 = arith.constant 1.000000e+00 : f32
    // CHECK: %[[CST_FLT_B:.*]] = arith.constant
    %cf5 = arith.constant 5.000000e+00 : f32

    // CHECK : qmem.send_float %[[CST_FLT_A]] {remote = @[[BOBREMOTE]]}
    qnet.send_float %cf0 {remote = @Bob} : f32
    // CHECK : qmem.send_float %[[CST_FLT_B]] {remote = @[[BOBREMOTE]]}
    qnet.send_float %cf5 {remote = @Bob} : f32

    // CHECK: qmem.return
    qnet.return
  }
}