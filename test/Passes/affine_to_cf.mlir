// RUN: qoala-opt -sccp -lower-affine -convert-scf-to-cf -sccp %s | FileCheck %s

// CHECK:         module {
// CHECK-NEXT:      %cst = arith.constant dense<[0, 1, 2, 3, 4, 5]> : tensor<6xi32>
// CHECK-NEXT:      func.func @init_5(%arg0: tensor<6xi32>) attributes {qoala_func = "local_routine"} {
// CHECK-NEXT:        %c1 = arith.constant 1 : index
// CHECK-NEXT:        %c6 = arith.constant 6 : index
// CHECK-NEXT:        %c0 = arith.constant 0 : index
// CHECK-NEXT:        cf.br ^bb1(%c1 : index)
// CHECK-NEXT:      ^bb1(%0: index):  // 2 preds: ^bb0, ^bb2
// CHECK-NEXT:        %1 = arith.cmpi slt, %0, %c6 : index
// CHECK-NEXT:        cf.cond_br %1, ^bb2, ^bb3
// CHECK-NEXT:      ^bb2:  // pred: ^bb1
// CHECK-NEXT:        %extracted = tensor.extract %arg0[%c0] : tensor<6xi32>
// CHECK-NEXT:        %extracted_0 = tensor.extract %arg0[%0] : tensor<6xi32>
// CHECK-NEXT:        netqasm.init %extracted
// CHECK-NEXT:        netqasm.cnot %extracted, %extracted_0
// CHECK-NEXT:        %2 = arith.addi %0, %c1 : index
// CHECK-NEXT:        cf.br ^bb1(%2 : index)
// CHECK-NEXT:      ^bb3:  // pred: ^bb1
// CHECK-NEXT:        return
// CHECK-NEXT:      }
// CHECK-NEXT:      func.call @init_5(%cst) : (tensor<6xi32>) -> ()
// CHECK-NEXT:    }

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

  %vqubits = arith.constant dense<[0, 1, 2, 3, 4, 5]> : tensor<6xi32>
  func.call @init_5(%vqubits) : (tensor<6xi32>) -> ()
}