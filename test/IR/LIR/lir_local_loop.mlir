// RUN: qoala-opt %s

module {
    qoalahost.remote @Bob

    netqasm.local_routine @subrt1(%vqubits_tensor: tensor<5xi32>) -> tensor<5xi32> {
      %5 = arith.constant 5 : index
      %init_vqubits = affine.for %i = 0 to %5 iter_args(%tensor = %vqubits_tensor) -> tensor<5xi32> {
        %vq = netqasm.qalloc : i32
        netqasm.init %vq
        %updated_tensor = tensor.insert %vq into %tensor[%i] : tensor<5xi32>
        affine.yield %updated_tensor : tensor<5xi32>
      }
      netqasm.return %init_vqubits : tensor<5xi32>
    }

    netqasm.local_routine @subrt2(%vqubits: tensor<5xi32>, %num: i32, %denom: i32, %m: memref<5xi1>) -> memref<5xi1> {
      %5 = arith.constant 5 : index
      affine.for %i = 0 to %5 {
        %vq = tensor.extract %vqubits[%i] : tensor<5xi32>
        netqasm.rot_x %vq, %num, %denom
        %m_i = netqasm.measure %vq : i1
        memref.store %m_i, %m[%i] : memref<5xi1>
      }
      netqasm.return %m : memref<5xi1>
    }

    qoalahost.main_func @main() {
      %zero = arith.constant 0 : index
      // Allocate an empty tensor for the qubits:
      %vqubits = tensor.empty() : tensor<5xi32>
      qoalahost.call @subrt1(%vqubits) : (tensor<5xi32>) -> ()

      %floats1 = qoalahost.recv_floats {remote = @Bob, length = 1 : i32}: tensor<1xf32>
      %t1 = tensor.extract %floats1[%zero] : tensor<1xf32>
      // TODO: implement standard conversion code for float angle to int-tuple
      // %num, %denom = qoalahost.call @convert_float_to_num_and_denom(%t1) : tensor<1xf32> -> (i32, i32)
      %num = arith.constant 0 : i32
      %denom = arith.constant 4 : i32

      %m_init = memref.alloc() : memref<5xi1>
      %m = qoalahost.call @subrt2(%vqubits, %num, %denom, %m_init) : (tensor<5xi32>, i32, i32, memref<5xi1>) -> memref<5xi1>
    }
}