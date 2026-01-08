// RUN: qoala-opt %s | FileCheck %s

module {
    // CHECK: qnet.remote
    "qnet.remote"() {sym_name = "Bob"} : () -> ()

    %to_send_a = arith.constant 42 : i32
    %to_send_b = arith.constant 0 : i32

    // CHECK: qnet.send_int
    "qnet.send_int"(%to_send_a) {remote = @Bob} : (i32) -> ()
    // CHECK: qnet.send_int
    "qnet.send_int"(%to_send_b) {remote = @Bob} : (i32) -> ()

    // CHECK: qnet.recv_int
    %received_a = "qnet.recv_int"() {remote = @Bob} : () -> i32
    // CHECK: qnet.recv_int
    %received_b = "qnet.recv_int"() {remote = @Bob} : () -> i32

    %to_send_float_a = arith.constant 3.1415: f32
    %to_send_float_b = arith.constant 13.37 : f32

    // CHECK: qnet.send_float
    "qnet.send_float"(%to_send_float_a) {remote = @Bob} : (f32) -> ()
    // CHECK: qnet.send_float
    "qnet.send_float"(%to_send_float_b) {remote = @Bob} : (f32) -> ()

    // CHECK: qnet.recv_float
    %received_float_a = "qnet.recv_float"() {remote = @Bob} : () -> f32
    // CHECK: qnet.recv_float
    %received_float_b = "qnet.recv_float"() {remote = @Bob} : () -> f32
}