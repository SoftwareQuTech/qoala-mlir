// RUN: qoala-opt %s | FileCheck %s

module {
    "qmem.remote"() {sym_name = "Bob"} : () -> ()

    %qptrs = "qmem.qalloc"() {N = 10} : () -> tensor<10xi32>

// CHECK: qmem.eprs
    "qmem.eprs"(%qptrs) {N = 10 : i32, remote = @Bob} : (tensor<10xi32>) -> ()

// CHECK: qmem.eprs_measure
    %outcomes = "qmem.eprs_measure"(%qptrs) {N = 10 : i32, remote = @Bob} : (tensor<10xi32>) -> tensor<10xi1>
}