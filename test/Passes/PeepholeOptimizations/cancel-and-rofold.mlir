// RUN: qoala-opt %s --qnet-peephole-optimizations="normalize-angles=false" | FileCheck %s

module {
    qnet.func @cancel_simple() {
        // CHECK: %[[CST:.*]] = arith.constant 6.28318548 : f32
        // CHECK-NEXT: %[[q0:.*]] = qnet.new_qubit : !qnet.qubit
        // CHECK-NEXT: %[[q1:.*]] = qnet.rot_z %[[q0]], %[[CST]] : !qnet.qubit
        // CHECK-NEXT: qnet.return

        %cst = arith.constant 3.14159274 : f32
        %q1 = qnet.new_qubit : !qnet.qubit

        %q2 = qnet.rot_z %q1, %cst : !qnet.qubit

        %q3 = qnet.hadamard %q2 : !qnet.qubit
        %q4 = qnet.hadamard %q3 : !qnet.qubit

        %q5 = qnet.rot_z %q4, %cst : !qnet.qubit


        qnet.return
    }
}