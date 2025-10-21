// RUN: qoala-opt %s --qnet-dead-code-elimination | FileCheck %s

module {
    qnet.remote @Bob
    qnet.func @epr_unmeasured() {
        // CHECK: qnet.eprs
        %q1 = qnet.eprs  {remote = @Bob} : !qnet.qubit

        %pi = arith.constant 3.141592 : f32

        // CHECK-NOT: qnet.rot_x
        %q2 = qnet.rot_x %q1, %pi : !qnet.qubit
        // CHECK-NOT: qnet.hadamard
        %q3 = qnet.hadamard %q2 : !qnet.qubit

        qnet.return
    }
}