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

        qnet.return
    }
}