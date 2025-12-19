// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// CHECK: module
module {
  // CHECK: qremote.remote @[[REMOTEBOB:.*]]
  qmem.remote @Bob

  // CHECK: qoalahost.main_func @test_send_values()
  qmem.func @test_send_values() {
    // First block is the one containing the remote ID placeholder opertion
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.remote_id_ref  {classical = true, quantum = false, remote = @[[REMOTEBOB]]}
    // CHECK-NEXT: qoalahost.nop_term

    // tensor.from elements are folded into a "arith.constant dense", and placed at the start of the file
    // HOWEVER, when unfolding the send_ints/floats operations we will expand those "dense" attributes
    // in to declarations of constants, i.e., we will revert what constant propagation will do right before
    // we run the unfolding pass.
    // CHECK-DAG: %[[C0_INT:.*]] = arith.constant 0 : i32
    // CHECK-DAG: %[[C5_INT:.*]] = arith.constant 5 : i32
    // CHECK-DAG: %[[C1_FLOAT:.*]] = arith.constant 1.{{0*}}e+00 : f32
    // CHECK-DAG: %[[C5_FLOAT:.*]] = arith.constant 5.{{0*}}e+00 : f32

    %c0_i32 = arith.constant 0 : i32
    %c5_i32 = arith.constant 5 : i32

    %from_elements = tensor.from_elements %c0_i32, %c5_i32 : tensor<2xi32>

    // CHECK: qoalahost.send_int %[[C0_INT]] {remote = @[[REMOTEBOB]]} : i32
    // CHECK: qoalahost.send_int %[[C5_INT]] {remote = @[[REMOTEBOB]]} : i32
    qmem.send_ints %from_elements {remote = @Bob} : tensor<2xi32>

    %cst = arith.constant 1.000000e+00 : f32
    %cst_0 = arith.constant 5.000000e+00 : f32

    %from_elements_1 = tensor.from_elements %cst, %cst_0 : tensor<2xf32>

    // CHECK: qoalahost.send_float %[[C1_FLOAT]] {remote = @[[REMOTEBOB]]} : f32
    // CHECK: qoalahost.send_float %[[C5_FLOAT]] {remote = @[[REMOTEBOB]]} : f32
    qmem.send_floats %from_elements_1 {remote = @Bob} : tensor<2xf32>
    // CHECK: qoalahost.return
    qmem.return
  }
}