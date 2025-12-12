// RUN: qoala-opt --lower-qoala-hir-to-mir %s | FileCheck %s

module {
  //CHECK: qmem.remote @[[REMOTEBOB:.*]]
  qnet.remote @Bob
  // CHECK: qmem.func @test_lefover_unrealized_cast()
  qnet.func @test_lefover_unrealized_cast() {

    // CHECK: %[[QBIT0:.*]] = qmem.qalloc : i32
    // CHECK-NEXT: qmem.eprs %[[QBIT0]] {remote = @[[REMOTEBOB]]}
    %0 = qnet.eprs  {remote = @Bob} : !qnet.qubit

    // CHECK: %[[QBIT1:.*]] = qmem.qalloc : i32
    // CHECK-NEXT: qmem.init %[[QBIT1]]
    // CHECK-NOT: builtin.unrealized_conversion_cast
    %1 = qnet.new_qubit : !qnet.qubit

    // CHECK: qmem.hadamard %[[QBIT1]]
    %2 = qnet.hadamard %1 : !qnet.qubit

    // CHECK: qmem.cnot %[[QBIT1]], %[[QBIT0]]
    %qout0, %qout1 = qnet.cnot %1, %0 : !qnet.qubit, !qnet.qubit

    // CHECK: qmem.measure %[[QBIT0]] : i1
    %3 = qnet.measure %qout1 : i1
    // CHECK: qmem.measure %[[QBIT1]] : i1
    %4 = qnet.measure %qout0 : i1

    %c0_i32 = arith.constant 0 : i32
    %c1_i32 = arith.constant 1 : i32
    %from_elements = tensor.from_elements %c0_i32, %c1_i32 : tensor<2xi32>

    // CHECK : qmem.send_ints %[[.*]] {remote = @[[REMOTEBOB]]}
    qnet.send_ints %from_elements {remote = @Bob} : tensor<2xi32>

    // CHECK: %[[QBIT2:.*]] = qmem.qalloc : i32
    // CHECK-NEXT: qmem.eprs %[[QBIT2]] {remote = @[[REMOTEBOB]]}
    %5 = qnet.eprs  {remote = @Bob} : !qnet.qubit

    // CHECK: %[[RCVINTS:.*]] = qmem.recv_ints  {length = 2 : i32, remote = @[[REMOTEBOB]]} : tensor<2xi32>
    %6 = qnet.recv_ints  {length = 2 : i32, remote = @Bob} : tensor<2xi32>

    %cst = arith.constant 3.14159274 : f32

    // CHECK: qmem.rot_x %[[QBIT2]]
    %7 = qnet.rot_x %5, %cst : !qnet.qubit

    %cst_0 = arith.constant 3.14159274 : f32

    // CHECK: qmem.rot_z %[[QBIT2]]
    %8 = qnet.rot_z %7, %cst_0 : !qnet.qubit

    // CHECK: qmem.measure %[[QBIT2]] : i1
    %9 = qnet.measure %8 : i1
    qnet.return
  }
}