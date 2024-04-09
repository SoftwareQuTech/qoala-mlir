// RUN: qoala-opt %s

module {
  %zero = arith.constant 0 : index
  "qnet.remote"() {sym_name = "Bob"} : () -> ()

  %eprs = qnet.eprs {remote = @Bob} : !qnet.qubit

  %floats1 = qnet.recv_floats {"remote" = @Bob, "length" = 1 : i32} : tensor<1xf32>
  %t1 = tensor.extract %floats1[%zero] : tensor<1xf32>

  %q2 = qnet.rot_x %eprs, %t1 : !qnet.qubit

  %floats2 = qnet.recv_floats {"remote" = @Bob, "length" = 1 : i32} : tensor<1xf32>
  %t2 = tensor.extract %floats2[%zero] : tensor<1xf32>

  %q3 = qnet.rot_y %q2, %t2 : !qnet.qubit
  %m = qnet.measure %q3 : i1
}