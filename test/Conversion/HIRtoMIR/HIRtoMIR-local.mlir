// RUN: qoala-opt %s --lower-qoala-hir-to-mir | FileCheck %s

// CHECK: module

module {
  // CHECK: qmem.func @test_local_quantum_program()
  qnet.func @test_local_quantum_program() {
    // CHECK: %[[QBIT0:.*]] = qmem.qalloc : i32
    // CHECK-NEXT: qmem.init %[[QBIT0]]
    // CHECK-NOT: builtin.unrealized_conversion_cast
    %0 = qnet.new_qubit : !qnet.qubit

    // CHECK: %[[QBIT1:.*]] = qmem.qalloc : i32
    // CHECK-NEXT: qmem.init %[[QBIT1]]
    // CHECK-NOT: builtin.unrealized_conversion_cast
    %1 = qnet.new_qubit : !qnet.qubit

    %cst = arith.constant 2.120000e+01 : f32
    %cst_0 = arith.constant 0.0306796152 : f32

    // CHECK: qmem.rot_x %[[QBIT0]], %cst_0
    %2 = qnet.rot_x %0, %cst_0 : !qnet.qubit

    %cst_1 = arith.constant 1.050000e+01 : f32

    // CHECK: qmem.rot_y %[[QBIT0]], %cst_1
    %3 = qnet.rot_y %2, %cst_1 : !qnet.qubit

    // CHECK: qmem.rot_z %[[QBIT0]], %cst
    %4 = qnet.rot_z %3, %cst : !qnet.qubit

    // CHECK: qmem.hadamard %[[QBIT1]]
    %5 = qnet.hadamard %1 : !qnet.qubit

    // CHECK: qmem.x %[[QBIT1]]
    %6 = qnet.x %5 : !qnet.qubit

    // CHECK: qmem.y %[[QBIT1]]
    %7 = qnet.y %6 : !qnet.qubit

    // CHECK: qmem.z %[[QBIT1]]
    %8 = qnet.z %7 : !qnet.qubit

    // CHECK: qmem.cnot %[[QBIT0]], %[[QBIT1]]
    %qout0, %qout1 = qnet.cnot %4, %8 : !qnet.qubit, !qnet.qubit

    %cst_2 = arith.constant 2.710000e+00 : f32

    // CHECK: qmem.cz %[[QBIT0]], %[[QBIT1]]
    %qout0_1, %qout1_1 = qnet.cz %qout0, %qout1 : !qnet.qubit, !qnet.qubit

    // CHECK: qmem.crot_x %[[QBIT0]], %[[QBIT1]], %cst_2
    %qout0_2, %qout1_2 = qnet.crot_x %qout0_1, %qout1_1, %cst_2 : !qnet.qubit, !qnet.qubit

    // CHECK: qmem.measure %[[QBIT0]] : i1
    %9 = qnet.measure %qout0_2 : i1

    // CHECK: qmem.measure %[[QBIT1]] : i1
    %10 = qnet.measure %qout1_2 : i1

    // CHECK: qmem.return
    qnet.return
  }
}
