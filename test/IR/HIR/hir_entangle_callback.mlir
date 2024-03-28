// RUN: qoala-opt %s

module {
  %zero = arith.constant 0 : index
  "qnet.remote"() {sym_name = "Bob"} : () -> ()

  %eprs = qnet.eprs {N = 1 : i32, remote = @Bob} : tensor<1x!qnet.qubit>
  %q1 = tensor.extract %eprs[%zero] : tensor<1x!qnet.qubit>
  %q2 = qnet.hadamard %q1 : !qnet.qubit

  %floats1 = qnet.recv_floats {"remote" = @Bob, "length" = 1 : i32} : tensor<1xf32>
  %t1 = tensor.extract %floats1[%zero] : tensor<1xf32>

  %q3 = qnet.rot_x %q2, %t1 : !qnet.qubit
  %m = qnet.measure %q3 : i1
}