// RUN: qoala-opt %s --qnet-peephole-optimizations="two-pi-epsilon=1e-6" | FileCheck %s

module {
  qnet.func @elim_near_zero_angle_identity() {
    // CHECK: %[[Q0:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NOT: qnet.rot_z
    // CHECK-NEXT: qnet.return

    %q0 = qnet.new_qubit : !qnet.qubit
    %a = arith.constant 0.00000050 : f32
    %q1 = qnet.rot_z %q0, %a : !qnet.qubit
    qnet.return
  }
}
