// RUN: qoala-opt %s

module {
  %zero = arith.constant 0 : index
  "qmem.remote"() {sym_name = "Bob"} : () -> ()

  %qptrs = "qmem.qalloc"() {N = 1}: () -> tensor<1xi32>
  "qmem.eprs"(%qptrs) {N = 1 : i32, "remote" = @Bob} : (tensor<1xi32>) -> ()
  %qptr = tensor.extract %qptrs[%zero] : tensor<1xi32>
  "qmem.hadamard"(%qptr) : (i32) -> ()

  %floats = "qmem.recv_floats"() {"remote" = @Bob, "length" = 1 : i32} : () -> tensor<1xf32>
  %t1 = tensor.extract %floats[%zero] : tensor<1xf32>

  "qmem.rot_x"(%qptr, %t1) : (i32, f32) -> ()
  %m = "qmem.measure"(%qptr) : (i32) -> i1
}