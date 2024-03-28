// RUN: qoala-opt %s

module {
  "qoalahost.remote"() {sym_name = "Bob"} : () -> ()
  "netqasm.remote"() {sym_name = "Bob_NetQASM"} : () -> ()

  func.func @req1(%vqubit: i32) attributes { "qoala_func" = "request"} {
    %vqubits = tensor.from_elements %vqubit : tensor<1xi32>
    "netqasm.eprs"(%vqubits) {N = 1 : i32, remote = @Bob_NetQASM} : (tensor<1xi32>) -> ()
    return
  }

  func.func @subrt2(%vqubit: i32, %num: i32, %denom: i32) attributes { "qoala_func" = "local_routine"} {
    "netqasm.rot_x"(%vqubit, %num, %denom) : (i32, i32, i32) -> ()
    return
  }

  func.func @subrt3(%vqubit: i32, %num: i32, %denom: i32) -> i1
    attributes { "qoala_func" = "local_routine"} {
    "netqasm.rot_y"(%vqubit, %num, %denom) : (i32, i32, i32) -> ()
    %m = "netqasm.measure"(%vqubit) : (i32) -> i1
    return %m : i1
  }

  %zero = arith.constant 0 : index
  %vqubits = "qoalahost.qalloc"() {N = 1} : () -> tensor<1xi32>
  %vqubit = tensor.extract %vqubits[%zero] : tensor<1xi32>
  func.call @req1(%vqubit) : (i32) -> ()

  %floats1 = "qoalahost.recv_floats"() {remote = @Bob, length = 1 : i32}: () -> tensor<1xf32>
  %t1 = tensor.extract %floats1[%zero] : tensor<1xf32>
  // TODO: implement standard conversion code for float angle to int-tuple
  // %num1, %denom1 = func.call @conver_float_to_num_and_denom(%t1) : (f32) -> (i32, i32)
  %num1 = arith.constant 0 : i32
  %denom1 = arith.constant 4 : i32

  func.call @subrt2(%vqubit, %num1, %denom1) : (i32, i32, i32) -> ()

  %floats2 = "qoalahost.recv_floats"() {remote = @Bob, length = 1 : i32}: () -> tensor<1xf32>
  %t2 = tensor.extract %floats1[%zero] : tensor<1xf32>
  // %num2, %denom2 = func.call @conver_float_to_num_and_denom(%t1) : (f32) -> (i32, i32)
  %num2 = arith.constant 0 : i32
  %denom2 = arith.constant 4 : i32

  %m = func.call @subrt3(%vqubit, %num2, %denom2) : (i32, i32, i32) -> i1
}