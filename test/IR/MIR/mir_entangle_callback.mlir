// RUN: qoala-opt %s

module {
  %zero = arith.constant 0 : index
  "qmem.remote"() {sym_name = "Bob"} : () -> ()

  %qptr = "qmem.qalloc"() : () -> i32
  "qmem.eprs"(%qptr) {"remote" = @Bob} : (i32) -> ()
  "qmem.hadamard"(%qptr) : (i32) -> ()

  %floats = "qmem.recv_floats"() {"remote" = @Bob, "length" = 1 : i32} : () -> tensor<1xf32>
  %t1 = tensor.extract %floats[%zero] : tensor<1xf32>

  "qmem.rot_x"(%qptr, %t1) : (i32, f32) -> ()
  %m = "qmem.measure"(%qptr) : (i32) -> i1
}