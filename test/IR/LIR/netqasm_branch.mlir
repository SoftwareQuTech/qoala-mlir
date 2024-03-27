// RUN: qoala-opt %s | FileCheck %s

// CHECK: module

module {
  func.func @main() {
    ^b0:
      %vqubit = arith.constant 0 : i32
      %zero = arith.constant 0 : i32
      %one = arith.constant 1 : i32
      %num_iter = arith.constant 10: i32
      cf.br ^b1(%zero: i32)

    ^b1(%i: i32):
      "netqasm.init"(%vqubit) : (i32) -> ()
      "netqasm.hadamard"(%vqubit) : (i32) -> ()
      %continue = arith.cmpi slt, %i, %num_iter : i32
      %plus_one = arith.addi %i, %one : i32
      cf.cond_br %continue, ^b1(%plus_one : i32), ^b2

    ^b2:
      %m = "netqasm.measure"(%vqubit) : (i32) -> i1

      return
  }
}