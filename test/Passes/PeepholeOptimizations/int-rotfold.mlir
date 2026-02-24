// RUN: qoala-opt %s --qnet-peephole-optimizations="rotation-folding=true normalize-angles=false" | FileCheck %s

module {
  // Two consecutive rot_x_int on the same denominator are merged.
  // (3/8)π + (5/8)π  →  (8/8)π
  qnet.func @fold_x_same_denom() {
    // CHECK-LABEL: qnet.func @fold_x_same_denom
    // CHECK-NEXT:    %[[D:.*]] = arith.constant 3 : i32
    // CHECK-NEXT:    %[[N:.*]] = arith.constant 8 : i32
    // CHECK-NEXT:    %[[Q0:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT:    %{{.*}} = qnet.rot_x_int %[[Q0]], %[[N]], %[[D]] : !qnet.qubit
    // CHECK-NEXT:    qnet.return
    %q0 = qnet.new_qubit : !qnet.qubit
    %n1 = arith.constant 3 : i32
    %d1 = arith.constant 3 : i32
    %q1 = qnet.rot_x_int %q0, %n1, %d1 : !qnet.qubit
    %n2 = arith.constant 5 : i32
    %d2 = arith.constant 3 : i32
    %q2 = qnet.rot_x_int %q1, %n2, %d2 : !qnet.qubit
    qnet.return
  }

  // Two consecutive rot_y_int with different denominators are merged.
  // d_new = max(1,2) = 2;  n_new = 1*2^1 + 1*2^0 = 3
  // (1/2)π + (1/4)π  →  (3/4)π
  qnet.func @fold_y_diff_denom() {
    // CHECK-LABEL: qnet.func @fold_y_diff_denom
    // CHECK-NEXT:    %[[D:.*]] = arith.constant 2 : i32
    // CHECK-NEXT:    %[[N:.*]] = arith.constant 3 : i32
    // CHECK-NEXT:    %[[Q0:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT:    %{{.*}} = qnet.rot_y_int %[[Q0]], %[[N]], %[[D]] : !qnet.qubit
    // CHECK-NEXT:    qnet.return
    %q0 = qnet.new_qubit : !qnet.qubit
    %n1 = arith.constant 1 : i32
    %d1 = arith.constant 1 : i32
    %q1 = qnet.rot_y_int %q0, %n1, %d1 : !qnet.qubit
    %n2 = arith.constant 1 : i32
    %d2 = arith.constant 2 : i32
    %q2 = qnet.rot_y_int %q1, %n2, %d2 : !qnet.qubit
    qnet.return
  }

  // Two consecutive crot_x_int on same denominator are merged.
  // (2/8)π + (3/8)π  →  (5/8)π
  qnet.func @fold_crotx_int() {
    // CHECK-LABEL: qnet.func @fold_crotx_int
    // CHECK-NEXT:    %[[D:.*]] = arith.constant 3 : i32
    // CHECK-NEXT:    %[[N:.*]] = arith.constant 5 : i32
    // CHECK-NEXT:    %[[CTRL:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT:    %[[TGT:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT:    %{{.*}}, %{{.*}} = qnet.crot_x_int %[[CTRL]], %[[TGT]], %[[N]], %[[D]] : !qnet.qubit, !qnet.qubit
    // CHECK-NEXT:    qnet.return
    %ctrl = qnet.new_qubit : !qnet.qubit
    %tgt  = qnet.new_qubit : !qnet.qubit
    %n1   = arith.constant 2 : i32
    %d1   = arith.constant 3 : i32
    %c1, %t1 = qnet.crot_x_int %ctrl, %tgt, %n1, %d1 : !qnet.qubit, !qnet.qubit
    %n2   = arith.constant 3 : i32
    %d2   = arith.constant 3 : i32
    %c2, %t2 = qnet.crot_x_int %c1, %t1, %n2, %d2 : !qnet.qubit, !qnet.qubit
    qnet.return
  }

  // rot_x_int followed by rot_y_int must NOT be folded (different axes).
  qnet.func @no_fold_different_axes() {
    // CHECK-LABEL: qnet.func @no_fold_different_axes
    // CHECK:         qnet.rot_x_int
    // CHECK-NEXT:    qnet.rot_y_int
    %q0 = qnet.new_qubit : !qnet.qubit
    %n  = arith.constant 1 : i32
    %d  = arith.constant 2 : i32
    %q1 = qnet.rot_x_int %q0, %n, %d : !qnet.qubit
    %n2 = arith.constant 1 : i32
    %d2 = arith.constant 2 : i32
    %q2 = qnet.rot_y_int %q1, %n2, %d2 : !qnet.qubit
    qnet.return
  }
}
