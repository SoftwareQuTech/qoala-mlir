// RUN: qnet-opt --qnet-check-linear %s 2>&1 | FileCheck %s

module {
    %q0 = "qnet.new_qubit"() : () -> !qnet.qubit
    %q1 = "qnet.hadamard"(%q0) : (!qnet.qubit) -> !qnet.qubit
// CHECK: qnet.hadamard
    %q2 = "qnet.hadamard"(%q1) : (!qnet.qubit) -> !qnet.qubit
}