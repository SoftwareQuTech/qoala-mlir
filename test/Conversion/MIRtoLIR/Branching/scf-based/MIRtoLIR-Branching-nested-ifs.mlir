// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// CHECK: module
module {
  // CHECK: qremote.remote @[[REMOTEBOB:.*]]
  qmem.remote @Bob
  qmem.func @nested_ifs() {
    %true = arith.constant 1 : i1
    // CHECK: %[[CST:.+]] = arith.constant 4 : i32
    // CHECK: %[[CST_10:.+]] = arith.constant 10 : i32
    %c4_i32 = arith.constant 4 : i32
    // CHECK: %[[RECV_INT_0:.*]] = qoalahost.recv_int {remote = @[[REMOTEBOB]]} : i32
    %rec_int_0 = qmem.recv_int {remote = @Bob} : i32
    // CHECK: %[[CMP_RESULT:.+]] = arith.cmpi ne, %[[RECV_INT_0]], %[[CST]]
    %cmp = arith.cmpi ne, %rec_int_0, %c4_i32 : i32
    // From here down, we use cf.assert operations so they are not deleted by constant folding pass
    scf.if %cmp {
      cf.assert %true, "Branch true"
      %c10_i32 = arith.constant 10 : i32
      %rec_int_1 = qmem.recv_int {remote = @Bob} : i32
      %cmp_b = arith.cmpi ne, %rec_int_1, %c10_i32 : i32
      scf.if %cmp_b {
        cf.assert %true, "Branch true-true"
      } else {
        cf.assert %true, "Branch true-false"
      }
    } else {
      cf.assert %true, "Branch false"
    }
    // Since we can't capture forward references (block names that are defined later),
    // we need to assert the block namnes textually.
    // IMPORTANT: MIR to LIR *does* modify the list of blocks:
    // * Adds a first block (^bb0) that contains the qoalahost.remote_id_ref operation
    // * Splits this block (^bb0 in this MIR), into 3, since it isolates recv_int
    // All this implies that the arith.cmpi + cond_br instructions end up in the block ^bb3
    // So the jumps should be to ^bb4 and ^bb5
    // CHECK: cf.cond_br %[[CMP_RESULT]], ^bb4, ^bb10
    // CHECK-NEXT: ^bb4:
    // CHECK: cf.assert %true, "Branch true"
    // CHECK: ^bb5:
    // CHECK: %[[RECV_INT_1:.*]] = qoalahost.recv_int  {remote = @Bob} : i32
    // CHECK: ^bb6:
    // CHECK: %[[CMP_RESULT_B:.+]] = arith.cmpi ne, %[[RECV_INT_1]], %[[CST_10]] : i32
    // CHECK-NEXT: cf.cond_br %[[CMP_RESULT_B]], ^bb7, ^bb8
    // CHECK-NEXT: ^bb7:
    // CHECK: cf.assert %true, "Branch true-true"
    // CHECK-NEXT: cf.br ^bb9
    // CHECK-NEXT: ^bb8:
    // CHECK: cf.assert %true, "Branch true-false"
    // CHECK-NEXT: cf.br ^bb9
    // CHECK-NEXT: ^bb9:
    // CHECK: cf.br ^bb11
    // CHECK-NEXT: ^bb10:
    // CHECK: cf.assert %true, "Branch false"
    // CHECK-NEXT: cf.br ^bb11
    // CHECK-NEXT: ^bb11:
    // CHECK: cf.assert %true, "Join back"
    cf.assert %true, "Join back"
    qmem.return
  }
}