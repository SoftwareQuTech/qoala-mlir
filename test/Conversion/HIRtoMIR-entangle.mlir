// RUN: qoala-opt %s --convert-qnet-to-qmem | FileCheck %s

// CHECK: module

module {
  // CHECK: qmem.func @test_entangle_quantum_program() {
  qnet.func @test_entangle_quantum_program() {
    //CHECK: qmem.remote @Bob
    qnet.remote @Bob

    // CHECK: %[[QBIT0:.*]] = qmem.qalloc : i32
    // CHECK-NEXT: qmem.eprs %[[QBIT0]]
    %0 = qnet.eprs  {remote = @Bob} : !qnet.qubit

    // CHECK: %[[QBIT1:.*]] = qmem.qalloc : i32
    // CHECK-NEXT: qmem.eprs %[[QBIT1]]
    %1 = qnet.eprs  {remote = @Bob} : !qnet.qubit

    // CHECK: %[[QBIT2:.*]] = qmem.qalloc : i32
    // CHECK-NEXT: qmem.eprs %[[QBIT2]]
    %2 = qnet.eprs  {remote = @Bob} : !qnet.qubit

    // CHECK: %[[.*]] = qmem.recv_floats  {remote = @Bob} : tensor<2xf32>
    %3 = qnet.recv_floats  {remote = @Bob} : tensor<2xf32>

    %c0 = arith.constant 0 : index
    %extracted = tensor.extract %3[%c0] : tensor<2xf32>

    // CHECK: qmem.rot_x %[[QBIT2]], %extracted
    %4 = qnet.rot_x %2, %extracted : !qnet.qubit

    // CHECK: %[[.*]] = qmem.recv_floats  {remote = @Bob} : tensor<2xf32>
    %5 = qnet.recv_floats  {remote = @Bob} : tensor<2xf32>

    %c1 = arith.constant 1 : index
    %extracted_0 = tensor.extract %5[%c1] : tensor<2xf32>

    // CHECK: qmem.rot_x %[[QBIT2]], %extracted_0
    %6 = qnet.rot_y %4, %extracted_0 : !qnet.qubit

    // CHECK: qmem.measure %[[QBIT2]] : i1
    %7 = qnet.measure %6 : i1

    // CHECK: qmem.return
    qnet.return
  }
}