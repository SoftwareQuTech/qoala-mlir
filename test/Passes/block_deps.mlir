// RUN: qoala-opt %s --qoalahost-add-block-deps | FileCheck %s

module {
  qremote.remote @Bob
  netqasm.request_routine @__qoala_wrapper0() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0 {remote = @Bob}
    netqasm.return %0 : i32
  }
  netqasm.request_routine @__qoala_wrapper1() -> i32 {
    %1 = netqasm.qalloc  : i32
    netqasm.eprs %1 {remote = @Bob}
    netqasm.return %1 : i32
  }
  netqasm.local_routine @__qoala_wrapper2(%arg0: i32) -> (i32) {
    netqasm.rot_x %arg0 (3 : ui32, 2 : ui32)
    netqasm.return %arg0 : i32
  }
  netqasm.local_routine @__qoala_wrapper3(%arg0: i32, %arg1: i32) -> (i1, i1) {
    %0 = netqasm.measure %arg0 : i1
    %1 = netqasm.measure %arg1 : i1
    netqasm.return %0, %1 : i1, i1
  }
  qoalahost.main_func @test_add_block_deps() {
// CHECK: qoalahost.nop_meta "block_0" []
    %cstA = arith.constant 3 : i32
    %cstB = arith.constant 2 : i32
    %cstC = arith.constant 4 : i32
    %cstD = arith.constant dense<[5.000000e+00]> : tensor<1xf32>
    qoalahost.nop_term
    ^bb1:
// CHECK: qoalahost.nop_meta "block_1" ["block_0"]
        %resA = arith.addi %cstA, %cstB : i32
        qoalahost.nop_term
    ^bb2:
// CHECK: qoalahost.nop_meta "block_2" ["block_0"]
        %resB = arith.subi %cstA, %cstB : i32
        qoalahost.nop_term
    ^bb3:
// CHECK: qoalahost.nop_meta "block_3" []
        %recvA = qoalahost.recv_floats {length = 1 : i32, remote = @Bob} : tensor<1xf32>
    ^bb4:
// CHECK: qoalahost.nop_meta "block_4" []
        %q0 = qoalahost.call @__qoala_wrapper0() : () -> i32
    ^bb5:
// CHECK: qoalahost.nop_meta "block_5" ["block_0", "block_1", "block_2"]
        %resC = arith.muli %resA, %resB : i32
        %resD = arith.muli %cstC, %resC : i32
        qoalahost.nop_term
    ^bb6:
// CHECK: qoalahost.nop_meta "block_6" ["block_1", "block_2", "block_5"]
        %resE = arith.muli %resA, %resD : i32
        %resF = arith.muli %resE, %resB : i32
        qoalahost.nop_term
    ^bb7:
// CHECK: qoalahost.nop_meta "block_7" ["block_0", "block_3", "block_6"]
        %sendA = tensor.from_elements %cstA : tensor<1xi32>
        qoalahost.send_ints %sendA {remote = @Bob} : tensor<1xi32>
        %sendB = tensor.from_elements %resF : tensor<1xi32>
        qoalahost.send_ints %sendB {remote = @Bob} : tensor<1xi32>
        qoalahost.nop_term
    ^bb8:
// CHECK: qoalahost.nop_meta "block_8" ["block_7"]
        %recvB = qoalahost.recv_ints {length = 1 : i32, remote = @Bob} : tensor<1xi32>
    ^bb9:
// CHECK: qoalahost.nop_meta "block_9" ["block_4"]
        %q1 = qoalahost.call @__qoala_wrapper1() : () -> i32
    ^bb10:
// CHECK: qoalahost.nop_meta "block_10" ["block_4"]
        %q2 = qoalahost.call @__qoala_wrapper2(%q0) : (i32) -> (i32)
    ^bb11:
// CHECK: qoalahost.nop_meta "block_11" ["block_6", "block_8"]
        %sendC = tensor.from_elements %resE : tensor<1xi32>
        qoalahost.send_ints %sendC {remote = @Bob} : tensor<1xi32>
        qoalahost.nop_term
    ^bb12:
// CHECK: qoalahost.nop_meta "block_12" ["block_10", "block_9"]
        %m0, %m1 = qoalahost.call @__qoala_wrapper3(%q2, %q1) : (i32, i32) -> (i1, i1)
    ^bb13:
// CHECK: qoalahost.nop_meta "block_13" ["block_0", "block_11"]
        qoalahost.send_floats %cstD {remote = @Bob} : tensor<1xf32>
    qoalahost.return
  }
}
