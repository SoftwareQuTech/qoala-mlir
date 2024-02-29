// RUN: hir-opt --hir-check-linear %s 2>&1 | FileCheck %s

// CHECK: error: Result of operation is used more than once.

module {
    %q0 = "hir.new_qubit"() : () -> !hir.qubit
    %q1 = "hir.hadamard"(%q0) : (!hir.qubit) -> !hir.qubit
    %q2 = "hir.hadamard"(%q0) : (!hir.qubit) -> !hir.qubit
}