// RUN: qoala-opt %s --lower-qoala-hir-to-mir | FileCheck %s

// CHECK: module
module {
  //CHECK: qmem.remote @[[REMOTE:.*]]
  qnet.remote @client
  // CHECK: qmem.func @branching_yielding_val_i32()
  qnet.func @branching_yielding_val_i32() {
    // CHECK-NEXT: %[[RECV_A:.+]] = qmem.recv_int  {remote = @[[REMOTE]]} : i32
    %0 = qnet.recv_int  {remote = @client} : i32
    // CHECK-NEXT: %[[RECV_B:.+]] = qmem.recv_int  {remote = @[[REMOTE]]} : i32
    %1 = qnet.recv_int  {remote = @client} : i32
    // CHECK: %[[CST_1:.+]] = arith.constant 1 : i32
    %c1_i32 = arith.constant 1 : i32
    // CHECK: %[[CMP_RESULT:.+]] = arith.cmpi eq, %[[RECV_A]], %[[CST_1]]
    %2 = arith.cmpi eq, %0, %c1_i32 : i32
    // CHECK-NEXT: %[[IF_RES:.+]] = scf.if %[[CMP_RESULT]] -> (i32) {
    %3 = scf.if %2 -> (i32) {
      // CHECK-NEXT: %[[CST_10:.+]] = arith.constant 10 : i32
      %c10_i32 = arith.constant 10 : i32
      // CHECK-NEXT: %[[SUB_RES:.+]] = arith.subi %[[CST_10]], %[[RECV_B]] : i32
      %4 = arith.subi %c10_i32, %1 : i32
      // CHECK-NEXT: scf.yield %[[SUB_RES]] : i32
      scf.yield %4 : i32
    // Here we expect having an "else" branch. Despite there is a single yield
    // in the else branch, it *does* return a non-qubit value. So it should remain
    // after the scf.if rewrite.
    // CHECK-NEXT: } else {
    } else {
      // CHECK-NEXT: scf.yield %[[RECV_B]] : i32
      scf.yield %1 : i32
    // CHECK-NEXT: }
    }
    // CHECK-NEXT: qmem.return %[[IF_RES]] : i32
    qnet.return %3 : i32
  }
}