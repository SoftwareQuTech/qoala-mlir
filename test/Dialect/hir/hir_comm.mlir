// RUN: hir-opt %s | FileCheck %s

module {
// CHECK: hir.remote
    "hir.remote"() {sym_name = "Bob"} : () -> ()

// CHECK: hir.send_ints
    %to_send = arith.constant dense<[42, 1337]> : tensor<2xi32>
    "hir.send_ints"(%to_send) {remote = @Bob} : (tensor<2xi32>) -> ()
    
// CHECK: hir.recv_ints
    %received = "hir.recv_ints"() {remote = @Bob, length = 2 : i32} : () -> tensor<2xi32>

// CHECK: hir.send_floats
    %to_send_f = arith.constant dense<[3.1415, 13.37]> : tensor<2xf32>
    "hir.send_floats"(%to_send_f) {remote = @Bob} : (tensor<2xf32>) -> ()
    
// CHECK: hir.recv_floats
    %received_f = "hir.recv_floats"() {remote = @Bob, length = 2 : i32} : () -> tensor<2xf32>
}