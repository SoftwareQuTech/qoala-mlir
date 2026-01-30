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
    // CHECK-NEXT: %[[I32_VAL:.+]] = scf.if %[[CMP_RESULT]] -> (i32) {
    %2, %3 = scf.if %1 -> (!qnet.qubit, i32){
      // CHECK-NEXT: qmem.z %0
      %26 = qnet.z %0 : !qnet.qubit
      // CHECK-NEXT: %[[CST_10:.+]] = arith.constant 10 : i32
      %c10_i32 = arith.constant 10 : i32
      // CHECK-NEXT: scf.yield %[[CST_10]] : i32
      scf.yield %26, %c10_i32 : !qnet.qubit, i32
    // Since qubit type will dissapear after HIR->MIR lowering, this branch becomes
    // empty, hence we expect *not* having an "else" branch.
    // CHECK-NEXT: } else {
    } else {
      // CHECK-NEXT: %[[CST_0:.+]] = arith.constant 0 : i32
      %c0_i32 = arith.constant 0 : i32
      // CHECK-NEXT: scf.yield %[[CST_0]] : i32
      scf.yield %0, %c0_i32 : !qnet.qubit, i32
    // CHECK-NEXT: }
    }
    // This next instruction should *not* leave an unrealized_convertion_cast operation
    // CHECK-NEXT: %[[MEAS:.+]] = qmem.measure %[[QBIT0]] : i1
    %4 = qnet.measure %2 : i1
    // CHECK-NEXT: %[[ADD_RES:.+]] = arith.addi %[[I32_VAL]], %[[CST_7]] : i32
    %5 = arith.addi %3, %c7_i32 : i32
    qnet.return %5 : i32
  }
}
