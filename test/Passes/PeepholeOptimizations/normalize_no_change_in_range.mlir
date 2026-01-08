// RUN: qoala-opt %s --qnet-peephole-optimizations=normalize-angles | FileCheck %s

module {
  qnet.func @normalize_no_change_in_range() {
    // CHECK: %[[CST:.*]] = arith.constant 1.000000e+00 : f32
    // CHECK-NEXT: %[[Q0:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT: %[[Q1:.*]] = qnet.rot_z %[[Q0]], %[[CST]] : !qnet.qubit
    // CHECK-NEXT: qnet.return

    %q0 = qnet.new_qubit : !qnet.qubit
    %a = arith.constant 1.00000000 : f32
    %q1 = qnet.rot_z %q0, %a : !qnet.qubit
    qnet.return
  }
}
