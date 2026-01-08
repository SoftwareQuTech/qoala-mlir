// RUN: qoala-opt %s --qnet-peephole-optimizations=normalize-angles | FileCheck %s

module {
  qnet.func @normalize_exact_minus_pi_to_plus_pi() {
    // CHECK: %[[CST:.*]] = arith.constant 3.1415925 : f32
    // CHECK-NEXT: %[[Q0:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT: %[[Q1:.*]] = qnet.rot_y %[[Q0]], %[[CST]] : !qnet.qubit
    // CHECK-NEXT: qnet.return

    %q0 = qnet.new_qubit : !qnet.qubit
    %a = arith.constant -3.14159274 : f32
    %q1 = qnet.rot_y %q0, %a : !qnet.qubit
    qnet.return
  }
}
