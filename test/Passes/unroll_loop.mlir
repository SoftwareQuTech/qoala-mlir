// RUN: qoala-opt -affine-loop-unroll=unroll-full -sccp -remove-dead-values -cse %s | FileCheck %s

// CHECK:       module {
// CHECK-NEXT:   func.func @init_5(%arg0: tensor<6xi32>) attributes {qoala_func = "local_routine"} {
// CHECK-NEXT:     %c5 = arith.constant 5 : index
// CHECK-NEXT:     %c4 = arith.constant 4 : index
// CHECK-NEXT:     %c3 = arith.constant 3 : index
// CHECK-NEXT:     %c2 = arith.constant 2 : index
// CHECK-NEXT:     %c0 = arith.constant 0 : index
// CHECK-NEXT:     %c1 = arith.constant 1 : index
// CHECK-NEXT:     %extracted = tensor.extract %arg0[%c0] : tensor<6xi32>
// CHECK-NEXT:     %extracted_0 = tensor.extract %arg0[%c1] : tensor<6xi32>
// CHECK-NEXT:     "netqasm.init"(%extracted) : (i32) -> ()
// CHECK-NEXT:     "netqasm.cnot"(%extracted, %extracted_0) : (i32, i32) -> ()
// CHECK-NEXT:     %extracted_1 = tensor.extract %arg0[%c2] : tensor<6xi32>
// CHECK-NEXT:     "netqasm.init"(%extracted) : (i32) -> ()
// CHECK-NEXT:     "netqasm.cnot"(%extracted, %extracted_1) : (i32, i32) -> ()
// CHECK-NEXT:     %extracted_2 = tensor.extract %arg0[%c3] : tensor<6xi32>
// CHECK-NEXT:     "netqasm.init"(%extracted) : (i32) -> ()
// CHECK-NEXT:     "netqasm.cnot"(%extracted, %extracted_2) : (i32, i32) -> ()
// CHECK-NEXT:     %extracted_3 = tensor.extract %arg0[%c4] : tensor<6xi32>
// CHECK-NEXT:     "netqasm.init"(%extracted) : (i32) -> ()
// CHECK-NEXT:     "netqasm.cnot"(%extracted, %extracted_3) : (i32, i32) -> ()
// CHECK-NEXT:     %extracted_4 = tensor.extract %arg0[%c5] : tensor<6xi32>
// CHECK-NEXT:     "netqasm.init"(%extracted) : (i32) -> ()
// CHECK-NEXT:     "netqasm.cnot"(%extracted, %extracted_4) : (i32, i32) -> ()
// CHECK-NEXT:     return
// CHECK-NEXT:   }
// CHECK-NEXT:   %0 = "qoalahost.qalloc"() {N = 6 : i64} : () -> tensor<6xi32>
// CHECK-NEXT:   func.call @init_5(%0) : (tensor<6xi32>) -> ()
// CHECK-NEXT: }

module {
  func.func @init_5(%vqubits: tensor<6xi32>) -> ()
    attributes {"qoala_func" = "local_routine"} {

    %N = arith.constant 6 : index
    %0 = arith.constant 0 : index  // comm qubit

    affine.for %i = 1 to %N {
      %comm = tensor.extract %vqubits[%0] : tensor<6xi32>
      %vq = tensor.extract %vqubits[%i] : tensor<6xi32>
      "netqasm.init"(%comm) : (i32) -> ()
      "netqasm.cnot"(%comm, %vq) : (i32, i32) -> ()
    }
    return
  }
  
  %0 = arith.constant 0 : index
  %vqubits = "qoalahost.qalloc"() {N = 6} : () -> tensor<6xi32>
  func.call @init_5(%vqubits) : (tensor<6xi32>) -> ()

}
