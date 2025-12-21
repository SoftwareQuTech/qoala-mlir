// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// There is NO quantum instructions here, so we don't expect any "functionization" effect

// CHECK: module
module {
  // CHECK: qremote.remote @[[REMOTEBOB:.*]]
  qmem.remote @Bob

  // CHECK: qoalahost.main_func @test_remote_quantum_program()
  qmem.func @test_remote_quantum_program() {
    // First block is the one containing the remote ID placeholder opertion
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_0:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.remote_id_ref  {classical = true, quantum = false, remote = @[[REMOTEBOB]]}
    // CHECK-NEXT: qoalahost.nop_term

    // CHECK: ^[[BLK_1:.*]]:
    // tensor.from elements are folded into a "arith.constant dense", and placed at the start of the file
    // Despite this, when unfolding the recv/send operations, those "folded dense" constants are unfolded again.
    // CHECK-DAG: %[[C0_INT:.*]] = arith.constant 0 : i32
    // CHECK-DAG: %[[C5_INT:.*]] = arith.constant 5 : i32
    // CHECK-DAG: %[[C1_FLOAT:.*]] = arith.constant 1.{{0*}}e+00 : f32
    // CHECK-DAG: %[[C5_FLOAT:.*]] = arith.constant 5.{{0*}}e+00 : f32
    // CHECK-NEXT: qoalahost.nop_term

    // CHECK: ^[[BLK_2:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta
    // CHECK-NEXT: %[[REC_INT_0:.*]] = qoalahost.recv_int {remote = @[[REMOTEBOB]]} : i32

    // CHECK: ^[[BLK_3:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta
    // CHECK-NEXT: %[[REC_INT_1:.*]] = qoalahost.recv_int {remote = @[[REMOTEBOB]]} : i32
    %0 = qmem.recv_ints {length = 2 : i32, remote = @Bob} : tensor<2xi32>

    %c0_i32 = arith.constant 0 : i32
    %c5_i32 = arith.constant 5 : i32

    %from_elements = tensor.from_elements %c0_i32, %c5_i32 : tensor<2xi32>

    // CHECK: ^[[BLK_4:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta
    // CHECK-NEXT: qoalahost.send_int %[[C0_INT]] {remote = @[[REMOTEBOB]]} : i32
    // CHECK-NEXT: qoalahost.send_int %[[C5_INT]] {remote = @[[REMOTEBOB]]} : i32
    // CHECK-NEXT: qoalahost.nop_term
    qmem.send_ints %from_elements {remote = @Bob} : tensor<2xi32>

    // CHECK: ^[[BLK_5:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta
    // CHECK: %[[REC_FLOAT_0:.*]] = qoalahost.recv_float {remote = @[[REMOTEBOB]]} : f32

    // CHECK: ^[[BLK_6:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta
    // CHECK: %[[REC_FLOAT_1:.*]] = qoalahost.recv_float {remote = @[[REMOTEBOB]]} : f32
    %1 = qmem.recv_floats {length = 2 : i32, remote = @Bob} : tensor<2xf32>

    // CHECK: ^[[BLK_7:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta
    %cst = arith.constant 1.000000e+00 : f32
    %cst_0 = arith.constant 5.000000e+00 : f32

    %from_elements_1 = tensor.from_elements %cst, %cst_0 : tensor<2xf32>

    // CHECK-NEXT: qoalahost.send_float %[[C1_FLOAT]] {remote = @[[REMOTEBOB]]} : f32
    // CHECK-NEXT: qoalahost.send_float %[[C5_FLOAT]] {remote = @[[REMOTEBOB]]} : f32
    qmem.send_floats %from_elements_1 {remote = @Bob} : tensor<2xf32>
    // CHECK-NEXT: qoalahost.return
    qmem.return
  }
}