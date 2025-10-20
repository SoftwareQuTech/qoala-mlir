// RUN: qoala-opt %s --qnet-dead-code-elimination | FileCheck %s

module {
    qnet.func @local_unmeasured_qubit() {
        // CHECK-NOT: qnet.new_qubit
        %q1 = qnet.new_qubit : !qnet.qubit

        %pi = arith.constant 3.141592 : f32

        // CHECK-NOT: qnet.rot_x
        %q2 = qnet.rot_x %q1, %pi : !qnet.qubit

        // CHECK-NOT: qnet.hadamard
        %q3 = qnet.hadamard %q2 : !qnet.qubit

        qnet.return
    }
}