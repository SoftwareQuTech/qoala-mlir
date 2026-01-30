// RUN: qoala-opt %s --lower-qoala-hir-to-mir | FileCheck %s

// CHECK: module
module {
  //CHECK: qmem.remote @[[REMOTE:.*]]
  qnet.remote @Bob
  // CHECK: qmem.func @recapture_qubit_val()
  qnet.func @recapture_qubit_val() {
    // CHECK: %[[QBIT0:.*]] = qmem.qalloc : i32
    // CHECK-NEXT: qmem.eprs %[[QBIT0]] {remote = @[[REMOTE]]}
    %0 = qnet.eprs  {remote = @Bob} : !qnet.qubit
    // CHECK-NEXT: %[[RECV_A:.+]] = qmem.recv_int  {remote = @[[REMOTE]]} : i32
    %1 = qnet.recv_int  {remote = @Bob} : i32
    // CHECK-NEXT: %[[RECV_B:.+]] = qmem.recv_int  {remote = @[[REMOTE]]} : i32
    %2 = qnet.recv_int  {remote = @Bob} : i32
    // CHECK: %[[CST_1:.+]] = arith.constant 1 : i32
    %c1_i32 = arith.constant 1 : i32
    // CHECK: %[[CMP_RESULT:.+]] = arith.cmpi eq, %[[RECV_A]], %[[CST_1]]
    %3 = arith.cmpi eq, %1, %c1_i32 : i32
    // CHECK-NEXT: scf.if %[[CMP_RESULT]] {
    %4 = scf.if %3 -> (!qnet.qubit) {
      // CHECK-NEXT: qmem.z %[[QBIT0]]
      %8 = qnet.z %0 : !qnet.qubit
      scf.yield %8 : !qnet.qubit
      // The "else" branch will be empty, so the next thing we want to
      // assert if the closing of the "then" branch
    } else {
      scf.yield %0 : !qnet.qubit
    // CHECK-NEXT: }
    }
    // CHECK: %[[CST_1_B:.+]] = arith.constant 1 : i32
    %c1_i32_0 = arith.constant 1 : i32
    // CHECK: %[[CMP_RESULT:.+]] = arith.cmpi eq, %[[RECV_B]], %[[CST_1_B]]
    %5 = arith.cmpi eq, %2, %c1_i32_0 : i32
    // CHECK-NEXT: scf.if %[[CMP_RESULT]] {
    %6 = scf.if %5 -> (!qnet.qubit) {
      // The algorithm also traces back the fact that %4 (despite being a value
      // yielded from an scf.if), is an alias of %0
      // CHECK-NEXT: qmem.x %[[QBIT0]]
      %8 = qnet.x %4 : !qnet.qubit
      scf.yield %8 : !qnet.qubit
      // The "else" branch will be empty, so the next thing we want to
      // assert if the closing of the "then" branch
    } else {
      scf.yield %4 : !qnet.qubit
    // CHECK-NEXT: }
    }
    // CHECK: %[[MEAS:.+]] = qmem.measure %[[QBIT0]] : i1
    %7 = qnet.measure %6 : i1
    qnet.return
  }
}
