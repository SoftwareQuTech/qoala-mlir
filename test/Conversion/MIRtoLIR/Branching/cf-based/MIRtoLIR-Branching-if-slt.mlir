// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// CHECK: module
module {
  // CHECK: qremote.remote @[[REMOTEBOB:.*]]
  qmem.remote @Bob
  qmem.func @branching_eq() {
    %true = arith.constant 1 : i1
    // CHECK: %[[CST:.+]] = arith.constant 4 : i32
    %c4_i32 = arith.constant 4 : i32
    // CHECK: %[[RECV_INT_0:.*]] = qoalahost.recv_int {remote = @[[REMOTEBOB]]} : i32
    %rec_int_0 = qmem.recv_int {remote = @Bob} : i32
    // CHECK: %[[CMP_RESULT:.+]] = arith.cmpi slt, %[[RECV_INT_0]], %[[CST]]
    %cmp = arith.cmpi slt, %rec_int_0, %c4_i32 : i32
    // Since we can't capture forward references (block names that are defined later),
    // we need to assert the block namnes textually.
    // IMPORTANT: MIR to LIR *does* modify the list of blocks:
    // * Adds a first block (^bb0) that contains the qoalahost.remote_id_ref operation
    // * Splits this block (^bb0 in this MIR), into 3, since it isolates recv_int
    // All this implies that the arith.cmpi + cond_br instructions end up in the block ^bb3
    // So the jumps should be to ^bb4 and ^bb5
    // CHECK: cf.cond_br %[[CMP_RESULT]], ^bb4, ^bb5
    cf.cond_br %cmp, ^bb1, ^bb2
    // From here down, we use cf.assert operations so they are not deleted by constant folding pass
  ^bb1:  // This block becomes ^bb4
    cf.assert %true, "Branch true"
    // CHECK: cf.br ^bb6
    cf.br ^bb3
  ^bb2:  // This block becomes ^bb5
    cf.assert %true, "Branch false"
    // CHECK: cf.br ^bb6
    cf.br ^bb3
  ^bb3:  // This block becomes ^bb6
    cf.assert %true, "Join back"
    qmem.return
  }
}