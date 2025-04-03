// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// CHECK: module
module {
  // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)

  // CHECK: netqasm.local_routine @[[WRAPPER0:.*]]() -> i1
  // CHECK-NEXT: %[[REG0_0:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.init %[[REG0_0]]
  // CHECK-NEXT: netqasm.rot_x %[[REG0_0]] (3 : ui32, 2 : ui32)
  // CHECK-NEXT: %[[REG0_1:.*]] = netqasm.measure %[[REG0_0]] : i1
  // CHECK-NEXT: netqasm.return %[[REG0_1]] : i1

  // CHECK: qoalahost.main_func @test_local_quantum_program()
  qmem.func @test_local_quantum_program() {
    // CHECK: qoalahost.nop_meta "block_0" []
    %0 = qmem.qalloc : i32
    qmem.init %0

    // This first test case assumes that *ALL* data dependencies are declared _BEFORE_ the quantum calls
    // We don't check the existence of the constants; they are eliminated by the translation process
    %cst_0 = arith.constant 1.214122 : f32
    %cst_1 = arith.constant 1.142072 : f32

    // 1.214122 + 1.142072 = 2.356194 -> (n, e) = (3, 2)
    %cst = arith.addf %cst_0, %cst_1 : f32

    // CHECK-NEXT: %[[REG_MAIN2:.*]] = qoalahost.call @[[WRAPPER0]]() : () -> i1
    qmem.rot_x %0, %cst
    %1 = qmem.measure %0 : i1

    // CHECK: ^[[BLOCK_1:.*]]:
    // CHECK-NEXT: qoalahost.nop_meta "block_1" []
    // CHECK-NEXT: qoalahost.return
    qmem.return
  }
}