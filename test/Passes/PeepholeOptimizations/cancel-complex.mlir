// UNSUPPORTED: true
// RUN: qoala-opt %s --qnet-peephole-optimizations=hermitian-cancel | FileCheck %s

module {
    qnet.func @cancel_simple() {
        // CHECK: %0 = qnet.new_qubit : !qnet.qubit
        // CHECK-NEXT: %1 = qnet.new_qubit : !qnet.qub
        // CHECK-NEXT: qnet.return

        %q1 = qnet.new_qubit : !qnet.qubit
        %q2 = qnet.new_qubit : !qnet.qubit

        %q3, %q4 = qnet.cnot %q1, %q2 : !qnet.qubit, !qnet.qubit

        %q5 = qnet.hadamard %q3 : !qnet.qubit
        %q6 = qnet.hadamard %q5 : !qnet.qubit

        %q7, %q8 = qnet.cnot %q3, %q4 : !qnet.qubit, !qnet.qubit

        qnet.return
    }
}