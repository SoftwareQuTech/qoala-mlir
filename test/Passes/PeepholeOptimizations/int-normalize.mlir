// RUN: qoala-opt %s --qnet-peephole-optimizations="normalize-angles=true rotation-folding=false" | FileCheck %s

module {
  // n=5, d=2: angle = 5π/4 > π.  Reduced modulo 2^3=8 into (−4, 4] → n=−3.
  qnet.func @normalize_pos_overflow() {
    // CHECK-LABEL: qnet.func @normalize_pos_overflow
    // CHECK-DAG:     %[[N:.*]] = arith.constant -3 : i32
    // CHECK-DAG:     %[[D:.*]] = arith.constant 2 : i32
    // CHECK:         %[[Q0:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT:    %{{.*}} = qnet.rot_x_int %[[Q0]], %[[N]], %[[D]] : !qnet.qubit
    // CHECK-NEXT:    qnet.return
    %q0 = qnet.new_qubit : !qnet.qubit
    %n  = arith.constant 5 : i32
    %d  = arith.constant 2 : i32
    %q1 = qnet.rot_x_int %q0, %n, %d : !qnet.qubit
    qnet.return
  }

  // n=−5, d=2: angle = −5π/4 < −π.  Reduced modulo 8 into (−4, 4] → n=3.
  qnet.func @normalize_neg_overflow() {
    // CHECK-LABEL: qnet.func @normalize_neg_overflow
    // CHECK-DAG:     %[[N:.*]] = arith.constant 3 : i32
    // CHECK-DAG:     %[[D:.*]] = arith.constant 2 : i32
    // CHECK:         %[[Q0:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT:    %{{.*}} = qnet.rot_y_int %[[Q0]], %[[N]], %[[D]] : !qnet.qubit
    // CHECK-NEXT:    qnet.return
    %q0 = qnet.new_qubit : !qnet.qubit
    %n  = arith.constant -5 : i32
    %d  = arith.constant 2 : i32
    %q1 = qnet.rot_y_int %q0, %n, %d : !qnet.qubit
    qnet.return
  }

  // n=5, d=2 on a controlled rotation: same reduction as single-qubit case.
  qnet.func @normalize_crotx_int() {
    // CHECK-LABEL: qnet.func @normalize_crotx_int
    // CHECK-DAG:     %[[N:.*]] = arith.constant -3 : i32
    // CHECK-DAG:     %[[D:.*]] = arith.constant 2 : i32
    // CHECK:         %[[CTRL:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT:    %[[TGT:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT:    %{{.*}}, %{{.*}} = qnet.crot_x_int %[[CTRL]], %[[TGT]], %[[N]], %[[D]] : !qnet.qubit, !qnet.qubit
    // CHECK-NEXT:    qnet.return
    %ctrl = qnet.new_qubit : !qnet.qubit
    %tgt  = qnet.new_qubit : !qnet.qubit
    %n    = arith.constant 5 : i32
    %d    = arith.constant 2 : i32
    %c1, %t1 = qnet.crot_x_int %ctrl, %tgt, %n, %d : !qnet.qubit, !qnet.qubit
    qnet.return
  }

  // n=3, d=2: angle = 3π/4 ∈ (−π, π] — already canonical, no change.
  qnet.func @normalize_no_change() {
    // CHECK-LABEL: qnet.func @normalize_no_change
    // CHECK-NEXT:    %[[D:.*]] = arith.constant 2 : i32
    // CHECK-NEXT:    %[[N:.*]] = arith.constant 3 : i32
    // CHECK-NEXT:    %[[Q0:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT:    %{{.*}} = qnet.rot_x_int %[[Q0]], %[[N]], %[[D]] : !qnet.qubit
    // CHECK-NEXT:    qnet.return
    %q0 = qnet.new_qubit : !qnet.qubit
    %n  = arith.constant 3 : i32
    %d  = arith.constant 2 : i32
    %q1 = qnet.rot_x_int %q0, %n, %d : !qnet.qubit
    qnet.return
  }
}
