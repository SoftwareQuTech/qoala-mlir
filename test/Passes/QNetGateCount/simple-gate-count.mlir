// RUN: qoala-opt %s --qnet-show-analysis-gate-count | FileCheck %s
// CHECK:  [Gate Count]:
// CHECK:  - One-qubit gates: 1
// CHECK:  - Two-qubit gates: 2
// CHECK:  - Total gates: 3
// CHECK:  Detailed gate count:
// CHECK:  - One-qubit gates:
// CHECK:    * qubit[0]: 0
// CHECK:    * qubit[1]: 1
// CHECK:    * qubit[2]: 0
// CHECK:  - Two-qubit gates:
// CHECK:    * qubit[0]: 1
// CHECK:    * qubit[1]: 2
// CHECK:    * qubit[2]: 1
// CHECK:  -  All gates:
// CHECK:    * qubit[0]: 1
// CHECK:    * qubit[1]: 3
// CHECK:    * qubit[2]: 1

module {
    qnet.remote @Bob
    qnet.func @test_simple_gate_count() {
        %q1 = qnet.new_qubit : !qnet.qubit
        %q2 = qnet.new_qubit : !qnet.qubit

        %q3, %q4 = qnet.cnot %q1, %q2 : !qnet.qubit, !qnet.qubit

        %pi = arith.constant 3.141592 : f32

        %q5 = qnet.rot_x %q4, %pi : !qnet.qubit

        %q6 = qnet.eprs  {remote = @Bob} : !qnet.qubit

        %q7, %q8 = qnet.cnot %q6, %q5 : !qnet.qubit, !qnet.qubit

        %m = qnet.measure %q8 : i1

        qnet.return
    }
}