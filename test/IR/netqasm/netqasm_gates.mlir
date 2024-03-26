// RUN: netqasm-opt %s | FileCheck %s

// CHECK: module

module {
  memref.global @input : memref<10xi32>
  memref.global @output : memref<10xi32>

  func.func @main() {
    %sm_input = memref.get_global @input : memref<10xi32>
    %sm_output = memref.get_global @output : memref<10xi32>

    %vqubit = "netqasm.decl_vqubit"() {addr = 0 : i32} : () -> !netqasm.vqubit

    %num = arith.constant 2 : i32
    %denom = arith.constant 4 : i32

    "netqasm.init"(%vqubit) : (!netqasm.vqubit) -> ()
    "netqasm.hadamard"(%vqubit) : (!netqasm.vqubit) -> ()
    "netqasm.rot_x"(%vqubit, %num, %denom) : (!netqasm.vqubit, i32, i32) -> ()

    %m = "netqasm.measure"(%vqubit) : (!netqasm.vqubit) -> i32

    %index = arith.constant 0 : index
    memref.store %m, %sm_output[%index] : memref<10xi32>

    return
  }
}