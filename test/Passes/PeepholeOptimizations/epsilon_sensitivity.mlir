// RUN: qoala-opt %s --qnet-peephole-optimizations="two-pi-epsilon=1e-3" | FileCheck %s --check-prefix=LOOSE
// RUN: qoala-opt %s --qnet-peephole-optimizations="two-pi-epsilon=1e-9" | FileCheck %s --check-prefix=TIGHT

module {
  qnet.func @epsilon_sensitivity() {
    // LOOSE: %[[Q0:.*]] = qnet.new_qubit
    // LOOSE-NEXT: qnet.return
    // LOOSE-NOT: qnet.rot_z

    // TIGHT: qnet.rot_z

    %cst = arith.constant 6.283185 : f32
    %q0 = qnet.new_qubit : !qnet.qubit
    %q1 = qnet.rot_z %q0, %cst : !qnet.qubit

    qnet.return
  }
}
