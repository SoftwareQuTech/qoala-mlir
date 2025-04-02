// RUN: qoala-opt %s --qoalahost-add-block-deps

module {
  qremote.remote @Bob
  qoalahost.main_func @test_add_block_deps() {
    %cstA = arith.constant 3 : i32
    %cstB = arith.constant 2 : i32
    %cstC = arith.constant 4 : i32
    %cstD = arith.constant dense<[5.000000e+00]> : tensor<1xf32>
    qoalahost.nop_term
    ^bb1:
        %resA = arith.addi %cstA, %cstB : i32
        qoalahost.nop_term
    ^bb2:
        %resB = arith.subi %cstA, %cstB : i32
        qoalahost.nop_term
    ^bb3:
        %recvA = qoalahost.recv_floats {length = 1 : i32, remote = @Bob} : tensor<1xf32>
    ^bb4:
        %resC = arith.muli %resA, %resB : i32
        %resD = arith.muli %cstC, %resC : i32
        qoalahost.nop_term
    ^bb5:
        %resE = arith.muli %resA, %resD : i32
        %resF = arith.muli %resE, %resB : i32
        qoalahost.nop_term
    ^bb6:
        %sendA = tensor.from_elements %cstA : tensor<1xi32>
        qoalahost.send_ints %sendA {remote = @Bob} : tensor<1xi32>
        %sendB = tensor.from_elements %resF : tensor<1xi32>
        qoalahost.send_ints %sendB {remote = @Bob} : tensor<1xi32>
        qoalahost.nop_term
    ^bb7:
        %recvB = qoalahost.recv_ints {length = 1 : i32, remote = @Bob} : tensor<1xi32>
    ^bb8:
        %sendC = tensor.from_elements %resE : tensor<1xi32>
        qoalahost.send_ints %sendC {remote = @Bob} : tensor<1xi32>
        qoalahost.nop_term
    ^bb9:
        qoalahost.send_floats %cstD {remote = @Bob} : tensor<1xf32>
    qoalahost.return
  }
}
