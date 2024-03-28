// RUN: qoala-opt %s

module {
  "qmem.remote"() {sym_name = "Bob"} : () -> ()

  %0 = arith.constant 0 : index
  %5 = arith.constant 5 : index

  %qptrs = "qmem.qalloc"() {N = 5 : i32} : () -> tensor<5xi32>
  affine.for %i = 0 to %5 {
      %qptr = tensor.extract %qptrs[%i] : tensor<5xi32>
      "qmem.init"(%qptr) : (i32) -> ()
  }

  %floats1 = "qmem.recv_floats"() {"remote" = @Bob, "length" = 1 : i32} : () -> tensor<1xf32>
  %t1 = tensor.extract %floats1[%0] : tensor<1xf32>

  %m = memref.alloc() : memref<5xi1>
  affine.for %i = 0 to %5 {
      %qptr = tensor.extract %qptrs[%i] : tensor<5xi32>
      "qmem.rot_x"(%qptr, %t1) : (i32, f32) -> ()
      %m_i = "qmem.measure"(%qptr) : (i32) -> i1
      memref.store %m_i, %m[%i] : memref<5xi1>
  }
}