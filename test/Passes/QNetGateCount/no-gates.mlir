// RUN: qoala-opt %s --qnet-show-analysis-gate-count | FileCheck %s
// CHECK:  [Gate Count]:
// CHECK:  - One-qubit gates: 0
// CHECK:  - Two-qubit gates: 0
// CHECK:  - Total gates: 0
// CHECK:  Detailed gate count:
// CHECK:  - One-qubit gates:
// CHECK:    * qubit[0]: 0
// CHECK:    * qubit[1]: 0
// CHECK:  - Two-qubit gates:
// CHECK:    * qubit[0]: 0
// CHECK:    * qubit[1]: 0
// CHECK:  -  All gates:
// CHECK:    * qubit[0]: 0
// CHECK:    * qubit[1]: 0

module {
    qnet.remote @Bob
    qnet.func @test_no_gates_count() {
        %q0 = qnet.new_qubit : !qnet.qubit
        %q1 = qnet.eprs  {remote = @Bob} : !qnet.qubit
        %m0 = qnet.measure %q0 : i1
        %m1 = qnet.measure %q1 : i1
        qnet.return
    }
}