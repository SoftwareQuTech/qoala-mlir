// RUN: qoala-opt %s --qnet-peephole-optimizations="two-pi-epsilon=0.0 rotation-folding=false normalize-angles=false" | FileCheck %s

module {
  // n=0 on rot_x_int: zero rotation is identity, gate is eliminated.
  qnet.func @elim_zero_rot_x_int() {
    // CHECK-LABEL: qnet.func @elim_zero_rot_x_int
    // CHECK-NEXT:    %{{.*}} = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT:    qnet.return
    // CHECK-NOT:     qnet.rot_x_int
    %q0 = qnet.new_qubit : !qnet.qubit
    %n  = arith.constant 0 : i32
    %d  = arith.constant 5 : i32
    %q1 = qnet.rot_x_int %q0, %n, %d : !qnet.qubit
    qnet.return
  }

  // n=0 on rot_z_int: same elimination regardless of axis.
  qnet.func @elim_zero_rot_z_int() {
    // CHECK-LABEL: qnet.func @elim_zero_rot_z_int
    // CHECK-NEXT:    %{{.*}} = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT:    qnet.return
    // CHECK-NOT:     qnet.rot_z_int
    %q0 = qnet.new_qubit : !qnet.qubit
    %n  = arith.constant 0 : i32
    %d  = arith.constant 3 : i32
    %q1 = qnet.rot_z_int %q0, %n, %d : !qnet.qubit
    qnet.return
  }

  // n=0 on crot_x_int: both qubit wires are passed through unchanged.
  qnet.func @elim_zero_crotx_int() {
    // CHECK-LABEL: qnet.func @elim_zero_crotx_int
    // CHECK-NEXT:    %{{.*}} = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT:    %{{.*}} = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT:    qnet.return
    // CHECK-NOT:     qnet.crot_x_int
    %ctrl = qnet.new_qubit : !qnet.qubit
    %tgt  = qnet.new_qubit : !qnet.qubit
    %n    = arith.constant 0 : i32
    %d    = arith.constant 2 : i32
    %c1, %t1 = qnet.crot_x_int %ctrl, %tgt, %n, %d : !qnet.qubit, !qnet.qubit
    qnet.return
  }
}
