// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// CHECK: module
module {
  // CHECK: qremote.remote @[[REMOTEBOB:.*]]
  qmem.remote @Bob

  // CHECK: qoalahost.main_func @test_recv_ints_all_used()
  qmem.func @test_recv_ints_all_used() -> i32 {
    // First block is the one containing the remote ID placeholder opertion
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_0:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.remote_id_ref  {classical = true, quantum = false, remote = @[[REMOTEBOB]]}
    // CHECK-NEXT: qoalahost.nop_term

    // CHECK-NOT: arith.constant 0 : index
    // CHECK-NOT: arith.constant 1 : index
    // CHECK-NOT: arith.constant 2 : index

    %idx_0 = arith.constant 0 : index
    %idx_1 = arith.constant 1 : index
    %idx_2 = arith.constant 2 : index

    // The recv_ints will be expanded into multiple recv_int operations.
    // Each one of them will live in its own block

    %received_ints = qmem.recv_ints {length = 3 : i32, remote = @Bob} : tensor<3xi32>
    %recv_0 = tensor.extract %received_ints [%idx_0] : tensor<3xi32>
    %recv_1 = tensor.extract %received_ints [%idx_1] : tensor<3xi32>
    %recv_2 = tensor.extract %received_ints [%idx_2] : tensor<3xi32>
    // CHECK: qoalahost.blk_meta
    // CHECK-NEXT: %[[RECV_0:.*]] = qoalahost.recv_int {remote = @[[REMOTEBOB]]} : i32

    // CHECK: ^[[BLK_1:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta
    // CHECK-NEXT: %[[RECV_1:.*]] = qoalahost.recv_int {remote = @[[REMOTEBOB]]} : i32

    // CHECK: ^[[BLK_2:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta
    // CHECK-NEXT: %[[RECV_2:.*]] = qoalahost.recv_int {remote = @[[REMOTEBOB]]} : i32

    // Some extra operations to avoid folding deleting operations
    %1 = arith.addi %recv_0, %recv_2 : i32
    %2 = arith.subi %1, %recv_1 : i32
    qmem.return %2 : i32
    // CHECK: ^[[BLK_3:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta
    // CHECK-NEXT: %[[RES_1:.*]] = arith.addi %[[RECV_0]], %[[RECV_2]] : i32
    // CHECK-NEXT: %[[RES_2:.*]] = arith.subi %[[RES_1]], %[[RECV_1]] : i32
    // CHECK-NEXT: qoalahost.return %[[RES_2]] : i32
  }
}