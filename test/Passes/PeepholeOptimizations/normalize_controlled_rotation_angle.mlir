// RUN: qoala-opt %s --qnet-peephole-optimizations=normalize-angles | FileCheck %s

module {
  qnet.func @normalize_controlled_rotation_angle() {
    // CHECK: %[[CST:.*]] = arith.constant -2.28318524 : f32
    // CHECK-NEXT: %[[Q0:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT: %[[Q1:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT: %[[R0:.*]], %[[R1:.*]] = qnet.crot_x %[[Q0]], %[[Q1]], %[[CST]]
    // CHECK-NEXT: qnet.return

    %q0 = qnet.new_qubit : !qnet.qubit
    %q1 = qnet.new_qubit : !qnet.qubit
    %a = arith.constant 4.00000000 : f32
    %q2, %q3 = qnet.crot_x %q0, %q1, %a : !qnet.qubit, !qnet.qubit
    qnet.return
  }
}
