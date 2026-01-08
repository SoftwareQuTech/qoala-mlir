// RUN: qoala-opt %s --qnet-peephole-optimizations=normalize-angles | FileCheck %s

module {
  qnet.func @normalize_angle_lt_minus_pi() {
    // CHECK: %[[CST:.*]] = arith.constant 2.28318524 : f32
    // CHECK-NEXT: %[[Q0:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT: %[[Q1:.*]] = qnet.rot_x %[[Q0]], %[[CST]] : !qnet.qubit
    // CHECK-NEXT: qnet.return

    %q0 = qnet.new_qubit : !qnet.qubit
    // -4.0 rad normalized: -4.0 + 2pi = 2.28318524
    %a = arith.constant -4.00000000 : f32
    %q1 = qnet.rot_x %q0, %a : !qnet.qubit
    qnet.return
  }
}
