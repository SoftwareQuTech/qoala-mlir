// RUN: qoala-opt %s --lower-qoala-hir-to-mir | FileCheck %s

module {
  //CHECK: qmem.remote @[[REMOTEALICE:.*]]
  qnet.remote @Alice
  qnet.func @measure_outcome_used_by_extui() {
    // CHECK: %[[QBIT0:.*]] = qmem.qalloc : i32
    // CHECK-NEXT: qmem.init %[[QBIT0]]
    %0 = qnet.new_qubit : !qnet.qubit

    // CHECK: %[[CST0:.*]] = qmem.measure %[[QBIT0]] : i1
    %1 = qnet.measure %0 : i1

    // CHECK: %[[CST1:.*]] = arith.extui %[[CST0]] : i1 to i32
    %2 = arith.extui %1 : i1 to i32

    // CHECK: qmem.send_int %[[CST1]] {remote = @[[REMOTEALICE]]} : i32
    qnet.send_int %2 {remote = @Alice} : i32

    // CHECK: qmem.return
    qnet.return
  }
}
