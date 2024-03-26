// RUN: netqasm-opt %s | FileCheck %s

// CHECK: module

module {
  func.func @main() {
    ^b0:
      %vqubit = "netqasm.decl_vqubit"() {addr = 0 : i32} : () -> !netqasm.vqubit
      %zero = arith.constant 0 : i32
      %one = arith.constant 1 : i32
      %num_iter = arith.constant 10: i32
      cf.br ^b1(%zero: i32)

    ^b1(%i: i32):
      "netqasm.init"(%vqubit) : (!netqasm.vqubit) -> ()
      "netqasm.hadamard"(%vqubit) : (!netqasm.vqubit) -> ()
      %continue = arith.cmpi slt, %i, %num_iter : i32
      %plus_one = arith.addi %i, %one : i32
      cf.cond_br %continue, ^b1(%plus_one : i32), ^b2

    ^b2:
      %m = "netqasm.measure"(%vqubit) : (!netqasm.vqubit) -> i32

      return
  }
}