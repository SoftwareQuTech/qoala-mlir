// RUN: qoala-opt %s --qnet-peephole-optimizations=normalize-angles | FileCheck %s

module {
  qnet.func @normalize_angle_gt_pi() {
    // CHECK: %[[CST:.*]] = arith.constant -2.28318524 : f32
    // CHECK-NEXT: %[[Q0:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT: %[[Q1:.*]] = qnet.rot_z %[[Q0]], %[[CST]] : !qnet.qubit
    // CHECK-NEXT: qnet.return

    %q0 = qnet.new_qubit : !qnet.qubit
    // 3.5pi = 10.995574..., normalized: -0.5pi = -1.570796...
    // We’ll use explicit decimal: 3.5pi ~= 10.995574, but pick a simpler one:
    // 4.0 rad is > pi; normalized: 4.0 - 2pi = -2.28318548
    %a = arith.constant 4.00000000 : f32
    %q1 = qnet.rot_z %q0, %a : !qnet.qubit
    qnet.return
  }
}
