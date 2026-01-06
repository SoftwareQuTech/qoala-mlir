// RUN: qoala-opt %s --qnet-peephole-optimizations=pauli-to-rotations | FileCheck %s

module {
    qnet.func @pauli_and_rotfold() {
        // CHECK: %[[CST:.*]] = arith.constant 6.28318548 : f32
        // CHECK-NEXT: %[[q0:.*]] = qnet.new_qubit : !qnet.qubit
        // CHECK-NEXT: %[[q1:.*]] = qnet.rot_x %[[q0]], %[[CST]] : !qnet.qubit
        // CHECK-NEXT: qnet.return
        
        %cst = arith.constant 3.14159274 : f32
        %q = qnet.new_qubit : !qnet.qubit
        %q1 = qnet.x %q : !qnet.qubit
        %q2 = qnet.rot_x %q1, %cst : !qnet.qubit

        qnet.return
    }
}