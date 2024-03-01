// RUN: hir-opt --hir-check-linear %s 2>&1 | FileCheck %s

module {
    %q0 = "hir.new_qubit"() : () -> !hir.qubit
    %q1 = "hir.hadamard"(%q0) : (!hir.qubit) -> !hir.qubit
// CHECK: hir.hadamard
    %q2 = "hir.hadamard"(%q1) : (!hir.qubit) -> !hir.qubit
}