// RUN: qoala-opt %s

module {
  %zero = arith.constant 0 : index
  "qnet.remote"() {sym_name = "Bob"} : () -> ()

  %q1 = qnet.new_qubit : !qnet.qubit

  %floats1 = qnet.recv_floats {"remote" = @Bob, "length" = 1 : i32} : tensor<1xf32>
  %t1 = tensor.extract %floats1[%zero] : tensor<1xf32>

  %c1_i32 = arith.constant 1 : i32
  %c2_i32 = arith.constant 2 : i32

  %q2 = qnet.rot_z_int %q1, %c1_i32, %c2_i32 : !qnet.qubit

  %q3 = qnet.rot_x %q2, %t1 : !qnet.qubit

  %floats2 = qnet.recv_floats {"remote" = @Bob, "length" = 1 : i32} : tensor<1xf32>
  %t2 = tensor.extract %floats2[%zero] : tensor<1xf32>

  %q4 = qnet.rot_x %q3, %t2 : !qnet.qubit
  %m = qnet.measure %q4 : i1
}