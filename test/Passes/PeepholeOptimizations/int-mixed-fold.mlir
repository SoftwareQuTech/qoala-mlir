// RUN: qoala-opt %s --qnet-peephole-optimizations="rotation-folding=true normalize-angles=false" | FileCheck %s

// Float angle = π (3.14159274 f32) + int angle n=4, d=2 (= 4π/4 = π) → combined = 2π (6.28318548 f32).

module {
  // rot_x(π) followed by rot_x_int(4, 2): folded into rot_x(2π).
  qnet.func @fold_float_then_int() {
    // CHECK-LABEL: qnet.func @fold_float_then_int
    // CHECK-NEXT:    %[[CST:.*]] = arith.constant 6.28318548 : f32
    // CHECK-NEXT:    %[[Q0:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT:    %{{.*}} = qnet.rot_x %[[Q0]], %[[CST]] : !qnet.qubit
    // CHECK-NEXT:    qnet.return
    // CHECK-NOT:     qnet.rot_x_int
    %q0  = qnet.new_qubit : !qnet.qubit
    %cst = arith.constant 3.14159274 : f32
    %q1  = qnet.rot_x %q0, %cst : !qnet.qubit
    %n   = arith.constant 4 : i32
    %d   = arith.constant 2 : i32
    %q2  = qnet.rot_x_int %q1, %n, %d : !qnet.qubit
    qnet.return
  }

  // rot_x_int(4, 2) followed by rot_x(π): same result by commutativity of addition.
  qnet.func @fold_int_then_float() {
    // CHECK-LABEL: qnet.func @fold_int_then_float
    // CHECK-NEXT:    %[[CST:.*]] = arith.constant 6.28318548 : f32
    // CHECK-NEXT:    %[[Q0:.*]] = qnet.new_qubit : !qnet.qubit
    // CHECK-NEXT:    %{{.*}} = qnet.rot_x %[[Q0]], %[[CST]] : !qnet.qubit
    // CHECK-NEXT:    qnet.return
    // CHECK-NOT:     qnet.rot_x_int
    %q0  = qnet.new_qubit : !qnet.qubit
    %n   = arith.constant 4 : i32
    %d   = arith.constant 2 : i32
    %q1  = qnet.rot_x_int %q0, %n, %d : !qnet.qubit
    %cst = arith.constant 3.14159274 : f32
    %q2  = qnet.rot_x %q1, %cst : !qnet.qubit
    qnet.return
  }

  // rot_x (float) followed by rot_y_int: different axes, must NOT be folded.
  qnet.func @no_fold_mixed_different_axes() {
    // CHECK-LABEL: qnet.func @no_fold_mixed_different_axes
    // CHECK:         qnet.rot_x
    // CHECK:         qnet.rot_y_int
    %q0  = qnet.new_qubit : !qnet.qubit
    %cst = arith.constant 3.14159274 : f32
    %q1  = qnet.rot_x %q0, %cst : !qnet.qubit
    %n   = arith.constant 1 : i32
    %d   = arith.constant 2 : i32
    %q2  = qnet.rot_y_int %q1, %n, %d : !qnet.qubit
    qnet.return
  }
}
