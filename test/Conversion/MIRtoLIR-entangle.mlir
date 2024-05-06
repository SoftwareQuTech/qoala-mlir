// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// CHECK: module

module {
  // CHECK: qoalahost.remote @[[REMOTEBOB:.*]]
  qmem.remote @Bob
  // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)

  // CHECK: netqasm.local_routine @[[WRAPPER0:.*]]() -> i32
  // CHECK-NEXT: %[[REG0_0:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.return %[[REG0_0]] : i32

  // CHECK: netqasm.request_routine @[[WRAPPER1:.*]](%[[ARG0_0:.*]]: i32)
  // CHECK-NEXT: netqasm.eprs %arg0 {remote = @[[REMOTEBOB]]}
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER2:.*]]() -> i32
  // CHECK-NEXT: %[[REG0_2:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.return %[[REG0_2]] : i32


  // CHECK: netqasm.request_routine @[[WRAPPER3:.*]](%[[ARG3_0:.*]]: i32)
  // CHECK-NEXT: netqasm.eprs %[[ARG3_0]] {remote = @[[REMOTEBOB]]}
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER4:.*]]() -> i32
  // CHECK-NEXT: %[[REG0_4:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.return %[[REG0_4]] : i32

  // CHECK: netqasm.request_routine @[[WRAPPER5:.*]](%[[ARG0_5:.*]]: i32) {
  // CHECK-NEXT: netqasm.eprs %[[ARG0_5]] {remote = @[[REMOTEBOB]]}
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER6:.*]](%[[ARG0_6:.*]]: i32, %[[ARG1_6:.*]]: i32, %[[ARG2_6:.*]]: i32) {
  // CHECK-NEXT: netqasm.rot_x %[[ARG0_6]], %[[ARG1_6]], %[[ARG2_6]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER7:.*]](%[[ARG0_7:.*]]: i32, %[[ARG1_7:.*]]: i32, %[[ARG2_7:.*]]: i32) {
  // CHECK-NEXT: netqasm.rot_y %[[ARG0_7]], %[[ARG1_7]], %[[ARG2_7]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER8:.*]](%[[ARG0_8:.*]]: i32) -> i1 {
  // CHECK-NEXT: %[[REG0_8:.*]] = netqasm.measure %[[ARG0_8]] : i1
  // CHECK-NEXT: netqasm.return %[[REG0_8]] : i1

  // CHECK: netqasm.local_routine @[[WRAPPER9:.*]]() -> i32
  // CHECK-NEXT: %[[REG0_9:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.return %[[REG0_9]] : i32

  // CHECK: netqasm.local_routine @[[WRAPPER10:.*]](%[[ARG0_10:.*]]: i32) -> i1 {
  // CHECK-NEXT: %[[REG0_10:.*]] = netqasm.eprs_measure %[[ARG0_10]] {remote = @[[REMOTEBOB]]} : i1
  // CHECK-NEXT: netqasm.return %[[REG0_10]] : i1

  // CHECK: qoalahost.main_func @test_entangle_quantum_program()
  qmem.func @test_entangle_quantum_program() {
    // CHECK: %[[REG_MAIN0:.*]] = qoalahost.call @[[WRAPPER0]]() : () -> i32
    %0 = qmem.qalloc : i32
    // CHECK: qoalahost.call @[[WRAPPER1]](%[[REG_MAIN0]]) : (i32) -> ()
    qmem.eprs %0 {remote = @Bob}
    // CHECK: %[[REG_MAIN1:.*]] = qoalahost.call @[[WRAPPER2]]() : () -> i32
    %1 = qmem.qalloc : i32
    // CHECK: qoalahost.call @[[WRAPPER3]](%[[REG_MAIN1]]) : (i32) -> ()
    qmem.eprs %1 {remote = @Bob}
    // CHECK: %[[REG_MAIN2:.*]] = qoalahost.call @[[WRAPPER4]]() : () -> i32
    %2 = qmem.qalloc : i32
    // CHECK: qoalahost.call @[[WRAPPER5]](%[[REG_MAIN2]]) : (i32) -> ()
    qmem.eprs %2 {remote = @Bob}
    // CHECK: qoalahost.recv_floats {remote = @[[REMOTEBOB]]} : tensor<2xf32>
    %3 = qmem.recv_floats  {length = 2 : i32, remote = @Bob} : tensor<2xf32>
    %c0 = arith.constant 0 : index
    %extracted = tensor.extract %3[%c0] : tensor<2xf32>
    // CHECK: %[[REG_MAIN4:.*]]:2 = qoalahost.call @__qoala_convert_float_angle(%extracted) : (f32) -> (i32, i32)
    // CHECK-NEXT: qoalahost.call @[[WRAPPER6]](%[[REG_MAIN2]], %[[REG_MAIN4]]#0, %[[REG_MAIN4]]#1) : (i32, i32, i32) -> ()
    qmem.rot_x %2, %extracted
    // CHECK: qoalahost.recv_floats {remote = @[[REMOTEBOB]]} : tensor<2xf32>
    %4 = qmem.recv_floats  {length = 2 : i32, remote = @Bob} : tensor<2xf32>
    %c1 = arith.constant 1 : index
    %extracted_0 = tensor.extract %4[%c1] : tensor<2xf32>
    // CHECK: %[[REG_MAIN6:.*]]:2 = qoalahost.call @__qoala_convert_float_angle(%extracted_0) : (f32) -> (i32, i32)
    // CHECK-NEXT: qoalahost.call @[[WRAPPER7]](%[[REG_MAIN2]], %[[REG_MAIN6]]#0, %[[REG_MAIN6]]#1) : (i32, i32, i32) -> ()
    qmem.rot_y %2, %extracted_0
    // CHECK: %[[REG_MAIN7:.*]] = qoalahost.call @[[WRAPPER8]](%[[REG_MAIN2]]) : (i32) -> i1
    %5 = qmem.measure %2 : i1
    // CHECK: %[[REG_MAIN8:.*]] = qoalahost.call @[[WRAPPER9]]() : () -> i32
    %6 = qmem.qalloc : i32
    // CHECK: %[[REG_MAIN9:.*]] = qoalahost.call @[[WRAPPER10]](%[[REG_MAIN8]]) : (i32) -> i1
    %7 = qmem.eprs_measure %6 {remote = @Bob} : i1
    // CHECK: qoalahost.return
    qmem.return
  }
}