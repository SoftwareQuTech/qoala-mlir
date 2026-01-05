// RUN: qoala-opt %s --qnet-peephole-optimizations=hermitian-cancel | FileCheck %s

module {
    qnet.func @cancel_simple() {
        // CHECK: %0 = qnet.new_qubit : !qnet.qubit
        // CHECK-NEXT: %1 = qnet.new_qubit : !qnet.qub
        // CHECK-NEXT: qnet.return

        %q1 = qnet.new_qubit : !qnet.qubit
        %q2 = qnet.hadamard %q1 : !qnet.qubit
        %q3 = qnet.hadamard %q2 : !qnet.qubit

        %q4 = qnet.new_qubit : !qnet.qubit
        %q5, %q6 = qnet.cnot %q3, %q4 : !qnet.qubit, !qnet.qubit
        %q7, %q8 = qnet.cnot %q5, %q6 : !qnet.qubit, !qnet.qubit

        %q9, %q10 = qnet.cz %q7, %q8 : !qnet.qubit, !qnet.qubit
        %q11, %q12 = qnet.cz %q9, %q10 : !qnet.qubit, !qnet.qubit

        %q13 = qnet.x %q12 : !qnet.qubit
        %q14 = qnet.x %q13 : !qnet.qubit

        %q15 = qnet.y %q14 : !qnet.qubit
        %q16 = qnet.y %q15 : !qnet.qubit

        %q17 = qnet.z %q16 : !qnet.qubit
        %q18 = qnet.z %q17 : !qnet.qubit

        qnet.return
    }
}