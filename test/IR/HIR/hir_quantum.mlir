// RUN: qoala-opt %s | FileCheck %s

module {
// CHECK: qnet.new_qubit
    %q0 = "qnet.new_qubit"() : () -> !qnet.qubit
    %angle = arith.constant 3.141592 : f32

// CHECK: qnet.rot_x
// CHECK: qnet.rot_y
// CHECK: qnet.rot_z
    %q1 = "qnet.rot_x"(%q0, %angle) : (!qnet.qubit, f32) -> !qnet.qubit
    %q2 = "qnet.rot_y"(%q1, %angle) : (!qnet.qubit, f32) -> !qnet.qubit
    %q3 = "qnet.rot_z"(%q2, %angle) : (!qnet.qubit, f32) -> !qnet.qubit

// CHECK: qnet.hadamard
    %q4 = "qnet.hadamard"(%q2) : (!qnet.qubit) -> !qnet.qubit

    %qa0 = "qnet.new_qubit"() : () -> !qnet.qubit
    %qb0 = "qnet.new_qubit"() : () -> !qnet.qubit

// CHECK: qnet.cnot
// CHECK: qnet.cz
// CHECK: qnet.crot_x
    %qa1, %qb1 = "qnet.cnot"(%qa0, %qa1) : (!qnet.qubit, !qnet.qubit) -> (!qnet.qubit, !qnet.qubit)
    %qa2, %qb2 = "qnet.cz"(%qa0, %qa1) : (!qnet.qubit, !qnet.qubit) -> (!qnet.qubit, !qnet.qubit)
    %qa3, %qb3 = "qnet.crot_x"(%qa0, %qa1, %angle) : (!qnet.qubit, !qnet.qubit, f32) -> (!qnet.qubit, !qnet.qubit)

// CHECK: qnet.measure
    %m = "qnet.measure"(%qa3) : (!qnet.qubit) -> i1
}