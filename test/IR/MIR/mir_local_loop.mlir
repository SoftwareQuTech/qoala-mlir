// RUN: qoala-opt %s

module {
  "qmem.remote"() {sym_name = "Bob"} : () -> ()

  %c0 = arith.constant 0 : index
  %c5 = arith.constant 5 : index

  // In the meantime, qubit allocations and initializations are handled "one-by-one"...
  %qptr0 = "qmem.qalloc"() : () -> i32
  "qmem.init"(%qptr0) : (i32) -> ()
  %qptr1 = "qmem.qalloc"() : () -> i32
  "qmem.init"(%qptr0) : (i32) -> ()
  %qptr2 = "qmem.qalloc"() : () -> i32
  "qmem.init"(%qptr0) : (i32) -> ()
  %qptr3 = "qmem.qalloc"() : () -> i32
  "qmem.init"(%qptr0) : (i32) -> ()
  %qptr4 = "qmem.qalloc"() : () -> i32
  "qmem.init"(%qptr0) : (i32) -> ()

  // To test the loop, we will manually create a tensor with the alloc'd qubits
  %qptrs = tensor.from_elements %qptr0, %qptr1, %qptr2, %qptr3, %qptr4 : tensor<5xi32>

  %floats1 = "qmem.recv_floats"() {"remote" = @Bob, "length" = 1 : i32} : () -> tensor<1xf32>
  %t1 = tensor.extract %floats1[%c0] : tensor<1xf32>

  %m = memref.alloc() : memref<5xi1>
  affine.for %i = 0 to %c5 {
      %qptr = tensor.extract %qptrs[%i] : tensor<5xi32>
      "qmem.rot_x"(%qptr, %t1) : (i32, f32) -> ()
      %m_i = "qmem.measure"(%qptr) : (i32) -> i1
      memref.store %m_i, %m[%i] : memref<5xi1>
  }
}