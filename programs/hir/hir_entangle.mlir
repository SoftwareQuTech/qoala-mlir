// RUN: hir-opt %s | FileCheck %s

module {
    "hir.remote"() {sym_name = "Bob"} : () -> ()

// CHECK: hir.eprs
    %qubits = "hir.eprs"() {N = 10 : i32, remote = @Bob} : () -> tensor<10x!hir.qubit>

// CHECK: hir.eprs_measure
    %outcomes = "hir.eprs_measure"() {N = 10 : i32, remote = @Bob} : () -> tensor<10xi1>
}