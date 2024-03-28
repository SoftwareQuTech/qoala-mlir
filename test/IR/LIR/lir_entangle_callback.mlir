// RUN: qoala-opt %s

module {
  "qoalahost.remote"() {sym_name = "Bob"} : () -> ()
  "netqasm.remote"() {sym_name = "Bob_NetQASM"} : () -> ()

  func.func @req1(%vqubit: i32) attributes { "qoala_func" = "request"} {
    %vqubits = tensor.from_elements %vqubit : tensor<1xi32>
    "netqasm.eprs"(%vqubits) {N = 1 : i32, remote = @Bob_NetQASM} : (tensor<1xi32>) -> ()
    func.call @callback(%vqubit) : (i32) -> ()
    return
  }

  func.func @callback(%vqubit: i32) attributes { "qoala_func" = "local_routine"} {
    "netqasm.hadamard"(%vqubit) : (i32) -> ()
    return
  }

  func.func @subrt(%vqubit: i32, %num: i32, %denom: i32) -> i1
    attributes { "qoala_func" = "local_routine"} {
    "netqasm.rot_x"(%vqubit, %num, %denom) : (i32, i32, i32) -> ()
    %m = "netqasm.measure"(%vqubit) : (i32) -> i1
    return %m : i1
  }

  %zero = arith.constant 0 : index
  %vqubits = "qoalahost.qalloc"() {N = 1} : () -> tensor<1xi32>
  %vqubit = tensor.extract %vqubits[%zero] : tensor<1xi32>
  func.call @req1(%vqubit) : (i32) -> ()

  %floats = "qoalahost.recv_floats"() {remote = @Bob, length = 1 : i32}: () -> tensor<1xf32>
  %t1 = tensor.extract %floats[%zero] : tensor<1xf32>
  // TODO: implement standard conversion code for float angle to int-tuple
  // %num, %denom = func.call @conver_float_to_num_and_denom(%t1) : (f32) -> (i32, i32)
  %num = arith.constant 0 : i32
  %denom = arith.constant 4 : i32

  %m = func.call @subrt(%vqubit, %num, %denom) : (i32, i32, i32) -> i1
}