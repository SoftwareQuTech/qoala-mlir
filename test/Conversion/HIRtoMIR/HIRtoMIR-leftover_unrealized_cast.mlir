// RUN: qoala-opt --lower-qoala-hir-to-mir %s | FileCheck %s

module {
  //CHECK: qmem.remote @[[REMOTEBOB:.*]]
  qnet.remote @Bob
  // CHECK: qmem.func @test_leftover_unrealized_cast()
  qnet.func @test_leftover_unrealized_cast() {

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

    qnet.return
  }
}