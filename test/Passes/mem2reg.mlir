// RUN: qoala-opt %s | FileCheck %s

// CHECK: module

module {
  memref.global "private" constant @qubits : memref<2xi32> = dense<[0, 1]>

  func.func @myfunc() -> i32 {
    %ptr1 = memref.alloca() : memref<1xi32>
    %ptr2 = memref.alloca() : memref<1xi32>

    %val1 = arith.constant 3 : i32
    %val2 = arith.constant 4 : i32

    %zero = arith.constant 0 : index

    memref.store %val1, %ptr1[%zero] : memref<1xi32>
    memref.store %val2, %ptr2[%zero] : memref<1xi32>

    %loaded1 = memref.load %ptr1[%zero] : memref<1xi32>
    %loaded2 = memref.load %ptr2[%zero] : memref<1xi32>

    %result = arith.addi %loaded1, %loaded2 : i32
    return %result : i32
  }

  func.func @mySecondFunc() -> (i32) {
    %sm_input = memref.get_global @qubits : memref<2xi32>
    %zero = arith.constant 0 : index
    %value = memref.load %sm_input[%zero] : memref<2xi32>

    return %value : i32
  }
}