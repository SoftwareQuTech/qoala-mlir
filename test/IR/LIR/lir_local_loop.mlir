// RUN: qoala-opt %s

module {
  "qoalahost.remote"() {sym_name = "Bob"} : () -> ()

  func.func @subrt1(%vqubits: tensor<5xi32>) attributes {"qoala_func" = "local_routine"} {
    %5 = arith.constant 5 : index
    affine.for %i = 0 to %5 {
      %vq = tensor.extract %vqubits[%i] : tensor<5xi32>
      "netqasm.init"(%vq) : (i32) -> ()
    }
    return
  }

  func.func @subrt2(%vqubits: tensor<5xi32>, %num: i32, %denom: i32, %m: memref<5xi1>) -> memref<5xi1>
    attributes {"qoala_func" = "local_routine"} {
    %5 = arith.constant 5 : index
    affine.for %i = 0 to %5 {
      %vq = tensor.extract %vqubits[%i] : tensor<5xi32>
      "netqasm.rot_x"(%vq, %num, %denom) : (i32, i32, i32) -> ()
      %m_i = "netqasm.measure"(%vq) : (i32) -> i1
      memref.store %m_i, %m[%i] : memref<5xi1>
    }
    return %m : memref<5xi1>
  }

  %zero = arith.constant 0 : index
  %vqubits = "qoalahost.qalloc"() {N = 5} : () -> tensor<5xi32>
  func.call @subrt1(%vqubits) : (tensor<5xi32>) -> ()

  %floats1 = "qoalahost.recv_floats"() {remote = @Bob, length = 1 : i32}: () -> tensor<1xf32>
  %t1 = tensor.extract %floats1[%zero] : tensor<1xf32>
  // TODO: implement standard conversion code for float angle to int-tuple
  // %num, %denom = func.call @conver_float_to_num_and_denom(%t1) : (f32) -> (i32, i32)
  %num = arith.constant 0 : i32
  %denom = arith.constant 4 : i32

  %m_init = memref.alloc() : memref<5xi1>
  %m = func.call @subrt2(%vqubits, %num, %denom, %m_init) : (tensor<5xi32>, i32, i32, memref<5xi1>) -> memref<5xi1>
}