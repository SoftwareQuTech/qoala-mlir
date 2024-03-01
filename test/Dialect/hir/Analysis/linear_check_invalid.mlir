// RUN: hir-opt --hir-check-linear %s --verify-diagnostics

module {
    %q0 = "hir.new_qubit"() : () -> !hir.qubit
    %q1 = "hir.hadamard"(%q0) : (!hir.qubit) -> !hir.qubit  // expected-error {{quantum operation is used more than once}}
    %q2 = "hir.hadamard"(%q0) : (!hir.qubit) -> !hir.qubit  // expected-error {{quantum operation is used more than once}}
    %q3 = "hir.hadamard"(%q1) : (!hir.qubit) -> !hir.qubit
    %q4 = "hir.hadamard"(%q0) : (!hir.qubit) -> !hir.qubit  // expected-error {{quantum operation is used more than once}}
}