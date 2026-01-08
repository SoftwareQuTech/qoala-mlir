// RUN: qoala-opt %s --lower-qoala-hir-to-mir | FileCheck %s

// CHECK: module
module {
  // CHECK: qmem.func @test_recursive_rewrite_pattern_application()
  qnet.func @test_recursive_rewrite_pattern_application() {
    // CHECK: %[[QBIT0:.*]] = qmem.qalloc : i32
    // CHECK-NEXT: qmem.init %[[QBIT0]]
    // CHECK-NOT: builtin.unrealized_conversion_cast
    %0 = qnet.new_qubit : !qnet.qubit

    // CHECK: %[[QBIT1:.*]] = qmem.qalloc : i32
    // CHECK-NEXT: qmem.init %[[QBIT1]]
    // CHECK-NOT: builtin.unrealized_conversion_cast
    %1 = qnet.new_qubit : !qnet.qubit

    %cst = arith.constant 3.14159274 : f32
    %cst_1 = arith.constant 3.14159274 : f32
    %cst_0 = arith.constant 3.14159274 : f32
    %cst_2 = arith.constant 1.57079637 : f32
    %cst_3 = arith.constant 0.785398185 : f32

    // CHECK: qmem.rot_x %[[QBIT0]], %cst
    %2 = qnet.rot_x %0, %cst : !qnet.qubit

    // CHECK: qmem.rot_y %[[QBIT0]], %cst_1
    %3 = qnet.rot_y %2, %cst_0 : !qnet.qubit

    // CHECK: qmem.rot_z %[[QBIT0]], %cst_0
    %4 = qnet.rot_z %3, %cst_1 : !qnet.qubit

    // CHECK: qmem.rot_z %[[QBIT0]], %cst_2
    %5 = qnet.rot_z %4, %cst_2 : !qnet.qubit

    // CHECK: qmem.rot_z %[[QBIT0]], %cst_3
    %6 = qnet.rot_z %5, %cst_3 : !qnet.qubit

    // CHECK: qmem.cz %[[QBIT0]], %[[QBIT1]]
    %qout0, %qout1 = qnet.cz %6, %1 : !qnet.qubit, !qnet.qubit

    // CHECK: qmem.measure %[[QBIT0]] : i1
    %7 = qnet.measure %qout0 : i1

    // CHECK: qmem.measure %[[QBIT1]] : i1
    %8 = qnet.measure %qout1 : i1
    qnet.return
  }
}