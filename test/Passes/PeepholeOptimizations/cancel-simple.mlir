// RUN: qoala-opt %s --qnet-peephole-optimizations=hermitian-cancellation | FileCheck %s

module {
    qnet.func @cancel_simple() {
        %q1 = qnet.new_qubit : !qnet.qubit

        %q2 = qnet.hadamard %q1 : !qnet.qubit

        %q3 = qnet.hadamard %q2 : !qnet.qubit

        qnet.return
    }
}