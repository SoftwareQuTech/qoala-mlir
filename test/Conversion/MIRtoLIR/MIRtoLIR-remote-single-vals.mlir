// RUN: qoala-opt %s --lower-qoala-mir-to-lir=disable-unfold-comm-ops | FileCheck %s

// There is NO quantum instructions here, so we don't expect any "functionization" effect

// CHECK: module
module {
  // CHECK: qremote.remote @[[REMOTEBOB:.*]]
  qmem.remote @Bob

  // CHECK: qoalahost.main_func @test_remote_quantum_program()
  qmem.func @test_remote_quantum_program() {
    // CHECK: %[[C0_I32:.*]] = arith.constant 0 : i32
    // CHECK: %[[C5_I32:.*]] = arith.constant 5 : i32
    // CHECK: %[[C_FLT:.*]] = arith.constant 5.000000e+00 : f32
    // CHECK-NEXT: qoalahost.nop_term

    // CHECK: ^[[BLOCK_1:.*]]:
    // CHECK: %[[RECV_INT_0:.*]] = qoalahost.recv_int {remote = @[[REMOTEBOB]]} : i32
    %rec_int_0 = qmem.recv_int {remote = @Bob} : i32

    // CHECK: ^[[BLOCK_2:.*]]:
    // CHECK: %[[RECV_INT_1:.*]] = qoalahost.recv_int {remote = @[[REMOTEBOB]]} : i32
    %rec_int_1 = qmem.recv_int {remote = @Bob} : i32

    // CHECK: ^[[BLOCK_3:.*]]:
    // CHECK: %[[PREP_INT:.*]] = arith.addi %[[RECV_INT_1]], %[[C5_I32]] : i32
    %c0_i32 = arith.constant 0 : i32
    %c5_i32 = arith.constant 5 : i32
    %prep_int = arith.addi %c5_i32, %rec_int_1 : i32

    // CHECK: qoalahost.send_int %[[C0_I32]] {remote = @[[REMOTEBOB]]} : i32
    // CHECK: qoalahost.send_int %[[PREP_INT]] {remote = @[[REMOTEBOB]]} : i32
    qmem.send_int %c0_i32 {remote = @Bob} : i32
    qmem.send_int %prep_int {remote = @Bob} : i32
    // CHECK-NEXT: qoalahost.nop_term

    // CHECK: ^[[BLOCK_4:.*]]:
    // CHECK: %[[RECV_FLOAT_0:.*]] = qoalahost.recv_float {remote = @[[REMOTEBOB]]} : f32
    %rec_float_0= qmem.recv_float {remote = @Bob} : f32

    // CHECK: ^[[BLOCK_5:.*]]:
    // CHECK: %[[RECV_FLOAT_1:.*]] = qoalahost.recv_float {remote = @[[REMOTEBOB]]} : f32
    %rec_float_1= qmem.recv_float {remote = @Bob} : f32

    // CHECK: ^[[BLOCK_6:.*]]:
    %cst = arith.constant 1.000000e+00 : f32
    %cst_0 = arith.constant 5.000000e+00 : f32

    // CHECK: %[[PREP_FLT:.*]] = arith.addf %[[RECV_FLOAT_0]], %[[C_FLT]] : f32
    %prep_float = arith.addf %cst_0, %rec_float_0 : f32

    // CHECK: qoalahost.send_float %[[C_FLT]] {remote = @[[REMOTEBOB]]} : f32
    // CHECK: qoalahost.send_float %[[PREP_FLT]] {remote = @[[REMOTEBOB]]} : f32
    qmem.send_float %cst_0 {remote = @Bob} : f32
    qmem.send_float %prep_float {remote = @Bob} : f32
    // CHECK: qoalahost.return
    qmem.return
  }
}