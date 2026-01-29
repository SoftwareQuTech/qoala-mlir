// RUN: qoala-opt %s --lower-qoala-hir-to-mir | FileCheck %s

// CHECK: module
module {
  //CHECK: qmem.remote @[[REMOTE:.*]]
  qnet.remote @client
  // CHECK: qmem.func @branching_yielding_single_qubit_val()
  qnet.func @branching_yielding_single_qubit_val() {
    // CHECK: %[[QBIT0:.*]] = qmem.qalloc : i32
    // CHECK-NEXT: qmem.eprs %[[QBIT0]] {remote = @[[REMOTE]]}
    %0 = qnet.eprs  {remote = @client} : !qnet.qubit
    // CHECK: %[[CST_4:.+]] = arith.constant 4 : i32
    %c4_i32 = arith.constant 4 : i32
    // CHECK: %[[CST_7:.+]] = arith.constant 7 : i32
    %c7_i32 = arith.constant 7 : i32
    // CHECK: %[[CMP_RESULT:.+]] = arith.cmpi eq, %[[CST_4]], %[[CST_7]]
    %1 = arith.cmpi eq, %c4_i32, %c7_i32 : i32
    // HIR -> MIR lowering makes a few semantic transformations to the IR:
    // * !qnet.qubit type becomes i32
    // * quantum operations now have memory side-effects
    // * In this sense, quamtum operations now DO NOT yield new values
    // Considering these last 2 points, we need to be careful when lowering
    // scf.if structures that yield !qnet.qubit values.
    // In MIR, these scf.if should not yield any !qnet.qubit, but rather rely
    // on the memory side-effects of the lowered quantum operations.
    // CHECK: scf.if %[[CMP_RESULT]] {
    %2 = scf.if %1 -> (!qnet.qubit){
      // CHECK-NEXT: qmem.z %0
      %26 = qnet.z %0 : !qnet.qubit
      scf.yield %26 : !qnet.qubit
    // Since qubit type will dissapear after HIR->MIR lowering, this branch becomes
    // empty, hence we expect *not* having an "else" branch.
    // CHECK-NEXT: } else {
    } else {
      scf.yield %0 : !qnet.qubit
    }
    // This next instruction should *not* leave an unrealized_convertion_cast operation
    // CHECK: %[[MEAS:.+]] = qmem.measure %[[QBIT0]] : i1
    %3 = qnet.measure %2 : i1
    qnet.return
  }
}
