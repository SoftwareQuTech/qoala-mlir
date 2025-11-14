// RUN: qoala-opt %s --qnet-show-analysis-gate-count | FileCheck %s
// CHECK:  [Gate Count]:
// CHECK:  - One-qubit gates: 2
// CHECK:  - Two-qubit gates: 4
// CHECK:  - Total gates: 6
// CHECK:  Detailed gate count:
// CHECK:  - One-qubit gates:
// CHECK:    * qubit[0]: 2
// CHECK:    * qubit[1]: 0
// CHECK:  - Two-qubit gates:
// CHECK:    * qubit[0]: 4
// CHECK:    * qubit[1]: 4
// CHECK:  -  All gates:
// CHECK:    * qubit[0]: 6
// CHECK:    * qubit[1]: 4

module {
    qnet.func @heavy_two_qubit() {
        %q0 = qnet.new_qubit : !qnet.qubit
        %q1 = qnet.hadamard %q0 : !qnet.qubit

        %q2 = qnet.new_qubit : !qnet.qubit
        %q3, %q4 = qnet.cnot %q2, %q1 : !qnet.qubit, !qnet.qubit
        %q5, %q6 = qnet.cnot %q4, %q3 : !qnet.qubit, !qnet.qubit

        %q7 = qnet.hadamard %q5 : !qnet.qubit

        %q8, %q9 = qnet.cz %q7, %q6 : !qnet.qubit, !qnet.qubit
        %q10, %q11 = qnet.cz %q8, %q9 : !qnet.qubit, !qnet.qubit

        qnet.return
    }
}
