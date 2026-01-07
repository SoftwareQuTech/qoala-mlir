// RUN: qoala-opt %s --qnet-peephole-optimizations="two-pi-epsilon=1e-6" | FileCheck %s

module {
  qnet.func @elim_2pi_enables_hermitian_cancel() {
    // CHECK-NOT: arith.constant 6.28318548 : f32
    // CHECK: %[[Q0:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NOT: qnet.hadamard
    // CHECK-NOT: qnet.rot_z
    // CHECK: qnet.return

    %cst = arith.constant 6.28318548 : f32
    %q0 = qnet.new_qubit : !qnet.qubit

    %q1 = qnet.hadamard %q0 : !qnet.qubit
    %q2 = qnet.rot_z %q1, %cst : !qnet.qubit
    %q3 = qnet.hadamard %q2 : !qnet.qubit

    qnet.return
  }
}
