// RUN: qoala-opt -affine-loop-unroll=unroll-full -sccp -remove-dead-values -cse %s | FileCheck %s

// CHECK:      func.func @init_5() attributes {qoala_func = "netqasm"} {
// CHECK-NEXT:   %c5_i32 = arith.constant 5 : i32
// CHECK-NEXT:   %c4_i32 = arith.constant 4 : i32
// CHECK-NEXT:   %c3_i32 = arith.constant 3 : i32
// CHECK-NEXT:   %c2_i32 = arith.constant 2 : i32
// CHECK-NEXT:   %c1_i32 = arith.constant 1 : i32
// CHECK-NEXT:   %c0_i32 = arith.constant 0 : i32
// CHECK-NEXT:   netqasm.init %c0_i32
// CHECK-NEXT:   netqasm.cnot %c0_i32, %c1_i32
// CHECK-NEXT:   netqasm.init %c0_i32
// CHECK-NEXT:   netqasm.cnot %c0_i32, %c2_i32
// CHECK-NEXT:   netqasm.init %c0_i32
// CHECK-NEXT:   netqasm.cnot %c0_i32, %c3_i32
// CHECK-NEXT:   netqasm.init %c0_i32
// CHECK-NEXT:   netqasm.cnot %c0_i32, %c4_i32
// CHECK-NEXT:   netqasm.init %c0_i32
// CHECK-NEXT:   netqasm.cnot %c0_i32, %c5_i32
// CHECK-NEXT:   return
// CHECK-NEXT: }

module {
  func.func @init_5() -> ()
    attributes {"qoala_func" = "netqasm"} {

    %N = arith.constant 6 : index
    %0 = arith.constant 0 : index  // comm qubit
    %vqubits = arith.constant dense<[0, 1, 2, 3, 4, 5]> : tensor<6xi32>

    affine.for %i = 1 to %N {
      %comm = tensor.extract %vqubits[%0] : tensor<6xi32>
      %vq = tensor.extract %vqubits[%i] : tensor<6xi32>
      "netqasm.init"(%comm) : (i32) -> ()
      "netqasm.cnot"(%comm, %vq) : (i32, i32) -> ()
    }
    return
  }
}
