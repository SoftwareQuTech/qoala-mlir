// RUN: qnet-opt --qnet-check-linear %s --verify-diagnostics

module {
    %q0 = "qnet.new_qubit"() : () -> !qnet.qubit
    %q1 = "qnet.hadamard"(%q0) : (!qnet.qubit) -> !qnet.qubit  // expected-error {{quantum operation is used more than once}}
    %q2 = "qnet.hadamard"(%q0) : (!qnet.qubit) -> !qnet.qubit  // expected-error {{quantum operation is used more than once}}
    %q3 = "qnet.hadamard"(%q1) : (!qnet.qubit) -> !qnet.qubit
    %q4 = "qnet.hadamard"(%q0) : (!qnet.qubit) -> !qnet.qubit  // expected-error {{quantum operation is used more than once}}
}