// RUN: qnet-opt %s | FileCheck %s

module {
    "qnet.remote"() {sym_name = "Bob"} : () -> ()

// CHECK: qnet.eprs
    %qubits = "qnet.eprs"() {N = 10 : i32, remote = @Bob} : () -> tensor<10x!qnet.qubit>

// CHECK: qnet.eprs_measure
    %outcomes = "qnet.eprs_measure"() {N = 10 : i32, remote = @Bob} : () -> tensor<10xi1>
}