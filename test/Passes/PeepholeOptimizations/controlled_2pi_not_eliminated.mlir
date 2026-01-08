// RUN: qoala-opt %s --qnet-peephole-optimizations="two-pi-epsilon=1e-6 normalize-angles=false" | FileCheck %s

module {
  qnet.func @controlled_2pi_not_eliminated() {
    // CHECK: %[[CST:.*]] = arith.constant 6.28318548 : f32
    // CHECK-NEXT: %[[Q0:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT: %[[Q1:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT: %[[R0:.*]], %[[R1:.*]] = qnet.crot_x %[[Q0]], %[[Q1]], %[[CST]] : !qnet.qubit, !qnet.qubit
    // CHECK-NEXT: qnet.return

    %cst = arith.constant 6.28318548 : f32
    %q0 = qnet.new_qubit : !qnet.qubit
    %q1 = qnet.new_qubit : !qnet.qubit

    %q2, %q3 = qnet.crot_x %q0, %q1, %cst : !qnet.qubit, !qnet.qubit

    qnet.return
  }
}
