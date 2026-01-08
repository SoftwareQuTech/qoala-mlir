// RUN: qoala-opt %s --qnet-peephole-optimizations=pauli-to-rotations | FileCheck %s

module {
    qnet.func @pauli_to_rotations() {
        // CHECK: %[[CST:.*]] = arith.constant 3.14159274 : f32
        // CHECK-NEXT: %[[q0:.*]] = qnet.new_qubit : !qnet.qubit
        // CHECK-NEXT: %[[q1:.*]] = qnet.rot_x %[[q0]], %[[CST]] : !qnet.qubit
        // CHECK-NEXT: %[[q2:.*]] = qnet.rot_y %[[q1]], %[[CST]] : !qnet.qubit
        // CHECK-NEXT: %[[q3:.*]] = qnet.rot_z %[[q2]], %[[CST]] : !qnet.qubit
        // CHECK-NEXT: qnet.return

        %q = qnet.new_qubit : !qnet.qubit
        %q1 = qnet.x %q : !qnet.qubit
        %q2 = qnet.y %q1 : !qnet.qubit
        %q3 = qnet.z %q2 : !qnet.qubit

        qnet.return
    }
}
