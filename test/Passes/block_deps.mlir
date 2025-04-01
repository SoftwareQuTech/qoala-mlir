// RUN: qoala-opt %s

module {
  qoalahost.main_func @test_add_block_deps() {
    %cstA = arith.constant 3 : i32
    %cstB = arith.constant 2 : i32
    %cstC = arith.constant 4 : i32
    qoalahost.nop_term
    ^bb1:
        %resA = arith.addi %cstA, %cstB : i32
        qoalahost.nop_term
    ^bb2:
        %resB = arith.subi %cstA, %cstB : i32
        qoalahost.nop_term
    ^bb3:
        %resC = arith.muli %resA, %resB : i32
        %resD = arith.muli %cstC, %resC : i32
        qoalahost.nop_term
    ^bb4:
        %resE = arith.muli %resA, %resD : i32
        %resF = arith.muli %resE, %resB : i32
    qoalahost.return
  }
}
