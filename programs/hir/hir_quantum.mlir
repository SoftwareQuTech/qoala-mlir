// RUN: hir-opt %s | FileCheck %s

module {
// CHECK: hir.new_qubit
    %q0 = "hir.new_qubit"() : () -> !hir.qubit
    %angle = arith.constant 3.141592 : f32

// CHECK: hir.rot_x
// CHECK: hir.rot_y
// CHECK: hir.rot_z
    %q1 = "hir.rot_x"(%q0, %angle) : (!hir.qubit, f32) -> !hir.qubit
    %q2 = "hir.rot_y"(%q1, %angle) : (!hir.qubit, f32) -> !hir.qubit
    %q3 = "hir.rot_z"(%q2, %angle) : (!hir.qubit, f32) -> !hir.qubit

// CHECK: hir.hadamard
    %q4 = "hir.hadamard"(%q2) : (!hir.qubit) -> !hir.qubit

    %qa0 = "hir.new_qubit"() : () -> !hir.qubit
    %qb0 = "hir.new_qubit"() : () -> !hir.qubit

// CHECK: hir.cnot
// CHECK: hir.cz
// CHECK: hir.crot_x
    %qa1, %qb1 = "hir.cnot"(%qa0, %qa1) : (!hir.qubit, !hir.qubit) -> (!hir.qubit, !hir.qubit)
    %qa2, %qb2 = "hir.cz"(%qa0, %qa1) : (!hir.qubit, !hir.qubit) -> (!hir.qubit, !hir.qubit)
    %qa3, %qb3 = "hir.crot_x"(%qa0, %qa1, %angle) : (!hir.qubit, !hir.qubit, f32) -> (!hir.qubit, !hir.qubit)

// CHECK: hir.measure
    %m = "hir.measure"(%qa3) : (!hir.qubit) -> i1
}