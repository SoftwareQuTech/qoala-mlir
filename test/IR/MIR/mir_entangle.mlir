// RUN: qoala-opt %s | FileCheck %s

module {
    "qmem.remote"() {sym_name = "Bob"} : () -> ()

    %qptrs = "qmem.qalloc_n"() {N = 10} : () -> memref<10xi32>

// CHECK: qmem.eprs
    "qmem.eprs"(%qptrs) {N = 10 : i32, remote = @Bob} : (memref<10xi32>) -> ()

// CHECK: qmem.eprs_measure
    %outcomes = "qmem.eprs_measure"(%qptrs) {N = 10 : i32, remote = @Bob} : (memref<10xi32>) -> tensor<10xi1>
}