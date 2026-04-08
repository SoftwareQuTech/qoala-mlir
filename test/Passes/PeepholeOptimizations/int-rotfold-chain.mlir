// RUN: qoala-opt %s --qnet-peephole-optimizations="rotation-folding=true normalize-angles=false" | FileCheck %s

// Verify that the greedy rewriter converges when folding a chain of more than
// two consecutive same-axis rotations.  Pairwise pattern application must be
// applied repeatedly until a single rotation remains.

module {
  // Three consecutive rot_x_int on the same denominator.
  // (1/2^2)π + (2/2^2)π + (3/2^2)π  →  (6/2^2)π
  qnet.func @fold_x_chain_three() {
    // CHECK-LABEL: qnet.func @fold_x_chain_three
    // CHECK-NEXT:    %[[D:.*]] = arith.constant 2 : i32
    // CHECK-NEXT:    %[[N:.*]] = arith.constant 6 : i32
    // CHECK-NEXT:    %[[Q0:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT:    %{{.*}} = qnet.rot_x_int %[[Q0]], %[[N]], %[[D]] : !qnet.qubit
    // CHECK-NEXT:    qnet.return
    %q0 = qnet.new_qubit : !qnet.qubit
    %n1 = arith.constant 1 : i32
    %d1 = arith.constant 2 : i32
    %q1 = qnet.rot_x_int %q0, %n1, %d1 : !qnet.qubit
    %n2 = arith.constant 2 : i32
    %d2 = arith.constant 2 : i32
    %q2 = qnet.rot_x_int %q1, %n2, %d2 : !qnet.qubit
    %n3 = arith.constant 3 : i32
    %d3 = arith.constant 2 : i32
    %q3 = qnet.rot_x_int %q2, %n3, %d3 : !qnet.qubit
    qnet.return
  }
}
