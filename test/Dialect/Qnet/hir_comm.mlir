// RUN: qnet-opt %s | FileCheck %s

module {
// CHECK: qnet.remote
    "qnet.remote"() {sym_name = "Bob"} : () -> ()

// CHECK: qnet.send_ints
    %to_send = arith.constant dense<[42, 1337]> : tensor<2xi32>
    "qnet.send_ints"(%to_send) {remote = @Bob} : (tensor<2xi32>) -> ()
    
// CHECK: qnet.recv_ints
    %received = "qnet.recv_ints"() {remote = @Bob, length = 2 : i32} : () -> tensor<2xi32>

// CHECK: qnet.send_floats
    %to_send_f = arith.constant dense<[3.1415, 13.37]> : tensor<2xf32>
    "qnet.send_floats"(%to_send_f) {remote = @Bob} : (tensor<2xf32>) -> ()
    
// CHECK: qnet.recv_floats
    %received_f = "qnet.recv_floats"() {remote = @Bob, length = 2 : i32} : () -> tensor<2xf32>
}