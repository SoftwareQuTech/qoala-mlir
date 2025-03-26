// UNSUPPORTED: true
// This test uses dynamic float values (received from other endpoint) which are not supported
// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// CHECK: module
module {
  // CHECK: qremote.remote @[[REMOTEBOB:.*]]
  qmem.remote @Bob
  // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)

  // CHECK: netqasm.request_routine @[[WRAPPER0:.*]]()
  // CHECK-NEXT: %[[REG0_0:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.eprs %[[REG0_0]] {remote = @[[REMOTEBOB]]}
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.request_routine @[[WRAPPER1:.*]]()
  // CHECK-NEXT: %[[REG0_1:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.eprs %[[REG0_1]] {remote = @[[REMOTEBOB]]}
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.request_routine @[[WRAPPER2:.*]]() -> i32
  // CHECK-NEXT: %[[REG0_2:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.eprs %[[REG0_2]] {remote = @[[REMOTEBOB]]}
  // CHECK-NEXT: netqasm.return %[[REG0_2]] : i32

  // CHECK: netqasm.local_routine @[[WRAPPER3:.*]](%[[ARG0_3:.*]]: i32, %[[ARG1_3:.*]]: i32, %[[ARG2_3:.*]]: i32)
  // CHECK-NEXT: netqasm.rot_x %[[ARG0_3]], %[[ARG1_3]], %[[ARG2_3]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER4:.*]](%[[ARG0_4:.*]]: i32, %[[ARG1_4:.*]]: i32, %[[ARG2_4:.*]]: i32)
  // CHECK-NEXT: netqasm.rot_y %[[ARG0_4]], %[[ARG1_4]], %[[ARG2_4]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.request_routine @[[WRAPPER5:.*]]() -> i1
  // CHECK-NEXT: %[[REG0_5:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: %[[REG1_5:.*]] = netqasm.eprs_measure %[[REG0_5]] {remote = @[[REMOTEBOB]]} : i1
  // CHECK-NEXT: netqasm.return %[[REG1_5]] : i1

  // CHECK: qoalahost.main_func @test_entangle_quantum_program()
  qmem.func @test_entangle_quantum_program() {
    // CHECK: qoalahost.call @[[WRAPPER0]]() : () -> ()
    %0 = qmem.qalloc : i32
    qmem.eprs %0 {remote = @Bob}

    // CHECK: ^[[BLOCK_1:.*]]:
    // CHECK-NEXT: qoalahost.call @[[WRAPPER1]]() : () -> ()
    %1 = qmem.qalloc : i32
    qmem.eprs %1 {remote = @Bob}

    // CHECK: ^[[BLOCK_2:.*]]:
    // CHECK-NEXT: %[[REG_MAIN0:.*]] = qoalahost.call @[[WRAPPER2]]() : () -> i32
    %2 = qmem.qalloc : i32
    qmem.eprs %2 {remote = @Bob}

    // CHECK: ^[[BLOCK_3:.*]]:
    // CHECK-NEXT: %[[REG_MAIN1:.*]] = qoalahost.recv_floats {length = 2 : i32, remote = @[[REMOTEBOB]]} : tensor<2xf32>
    %3 = qmem.recv_floats  {length = 2 : i32, remote = @Bob} : tensor<2xf32>

    // CHECK: ^[[BLOCK_4:.*]]:
    %c0 = arith.constant 0 : index
    // CHECK: %[[EXTRACTED:.*]] = tensor.extract %[[REG_MAIN1]]
    %extracted = tensor.extract %3[%c0] : tensor<2xf32>
    //CHECK-NEXT: qoalahost.nop_term

    // TODO - Decide if a call to __qoala_convert_float_angle must be in an isolated block (doesn't hurt)
    // CHECK: ^[[BLOCK_5:.*]]:
    // CHECK-NEXT: %[[REG_MAIN2:.*]]:2 = qoalahost.call @__qoala_convert_float_angle(%[[EXTRACTED]]) : (f32) -> (i32, i32)

    // CHECK: ^[[BLOCK_6:.*]]:
    // CHECK-NEXT: qoalahost.call @[[WRAPPER3]](%[[REG_MAIN0]], %[[REG_MAIN2]]#0, %[[REG_MAIN2]]#1) : (i32, i32, i32) -> ()
    qmem.rot_x %2, %extracted

    // CHECK: ^[[BLOCK_7:.*]]:
    // CHECK: %[[REG_MAIN3:.*]] = qoalahost.recv_floats {length = 2 : i32, remote = @[[REMOTEBOB]]} : tensor<2xf32>
    %4 = qmem.recv_floats  {length = 2 : i32, remote = @Bob} : tensor<2xf32>

    // CHECK: ^[[BLOCK_8:.*]]:
    %c1 = arith.constant 1 : index
    // CHECK: %[[EXTRACTED_0:.*]] = tensor.extract %[[REG_MAIN3]]
    %extracted_0 = tensor.extract %4[%c1] : tensor<2xf32>
    // CHECK-NEXT: qoalahost.nop_term

    // TODO - Decide if a call to __qoala_convert_float_angle must be in an isolated block (doesn't hurt)
    // CHECK: ^[[BLOCK_9:.*]]:
    // CHECK: %[[REG_MAIN4:.*]]:2 = qoalahost.call @__qoala_convert_float_angle(%[[EXTRACTED_0]]) : (f32) -> (i32, i32)

    // CHECK: ^[[BLOCK_10:.*]]:
    // CHECK-NEXT: qoalahost.call @[[WRAPPER4]](%[[REG_MAIN0]], %[[REG_MAIN4]]#0, %[[REG_MAIN4]]#1) : (i32, i32, i32) -> ()
    qmem.rot_y %2, %extracted_0

    // CHECK: ^[[BLOCK_11:.*]]:
    // CHECK-NEXT: %[[REG_MAIN6:.*]] = qoalahost.call @[[WRAPPER5]]() : () -> i1
    %6 = qmem.qalloc : i32
    %7 = qmem.eprs_measure %6 {remote = @Bob} : i1


    // CHECK: ^[[BLOCK_12:.*]]:
    // CHECK: qoalahost.return
    qmem.return
  }
}