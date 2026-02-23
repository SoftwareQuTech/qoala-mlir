// RUN: qoala-opt %s --qnet-peephole-optimizations="rotation-folding=true normalize-angles=false" | FileCheck %s

module {
  // Two consecutive rot_z_int where both n and d are runtime values are merged
  // into a single rotation.  The combined angle is computed via arith ops:
  //   d_new = max(e1, e2)
  //   n_new = n1 << (d_new-e1)  +  n2 << (d_new-e2)
  qnet.remote @client
  qnet.func @dynamic_fold_2() {
    // CHECK-LABEL: qnet.func @dynamic_fold_2
    // CHECK:         arith.maxsi
    // CHECK:         arith.addi
    // CHECK:         qnet.rot_z_int
    // CHECK-NOT:     qnet.rot_z_int
    %q0 = qnet.new_qubit : !qnet.qubit
    %n1 = qnet.recv_int {remote = @client} : i32
    %e1 = qnet.recv_int {remote = @client} : i32
    %q1 = qnet.rot_z_int %q0, %n1, %e1 : !qnet.qubit
    %n2 = qnet.recv_int {remote = @client} : i32
    %e2 = qnet.recv_int {remote = @client} : i32
    %q2 = qnet.rot_z_int %q1, %n2, %e2 : !qnet.qubit
    %m = qnet.measure %q2 : i1
    qnet.return %m : i1
  }

  // Two consecutive rot_z_int with a shared constant denominator and dynamic
  // numerators.  Both shift amounts are 0, so the maxsi and shli fold away,
  // leaving just arith.addi for the numerators.
  qnet.remote @client2
  qnet.func @dynamic_same_denom() {
    // CHECK-LABEL: qnet.func @dynamic_same_denom
    // CHECK-NOT:     arith.maxsi
    // CHECK:         arith.addi
    // CHECK:         qnet.rot_z_int
    // CHECK-NOT:     qnet.rot_z_int
    %d  = arith.constant 3 : i32
    %q0 = qnet.new_qubit : !qnet.qubit
    %n1 = qnet.recv_int {remote = @client2} : i32
    %q1 = qnet.rot_z_int %q0, %n1, %d : !qnet.qubit
    %n2 = qnet.recv_int {remote = @client2} : i32
    %q2 = qnet.rot_z_int %q1, %n2, %d : !qnet.qubit
    %m = qnet.measure %q2 : i1
    qnet.return %m : i1
  }

  // rot_z_int followed by rot_x_int must NOT be folded (different axes):
  // both operations must remain in the output.
  qnet.remote @client3
  qnet.func @dynamic_no_fold_different_axes() {
    // CHECK-LABEL: qnet.func @dynamic_no_fold_different_axes
    // CHECK:         qnet.rot_z_int
    // CHECK:         qnet.rot_x_int
    %q0 = qnet.new_qubit : !qnet.qubit
    %n1 = qnet.recv_int {remote = @client3} : i32
    %e1 = qnet.recv_int {remote = @client3} : i32
    %q1 = qnet.rot_z_int %q0, %n1, %e1 : !qnet.qubit
    %n2 = qnet.recv_int {remote = @client3} : i32
    %e2 = qnet.recv_int {remote = @client3} : i32
    %q2 = qnet.rot_x_int %q1, %n2, %e2 : !qnet.qubit
    %m = qnet.measure %q2 : i1
    qnet.return %m : i1
  }
}
