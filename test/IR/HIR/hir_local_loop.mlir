// RUN: qoala-opt %s

module {
  "qnet.remote"() {sym_name = "Bob"} : () -> ()

  %qubits_init = tensor.empty() : tensor<5x!qnet.qubit>
  %0 = arith.constant 0: index
  %5 = arith.constant 5 : index

  %qubits = affine.for %i = 0 to %5
      iter_args(%qubits_iter = %qubits_init) -> tensor<5x!qnet.qubit> {
    %q = qnet.new_qubit : !qnet.qubit
    %qubits_updated = tensor.insert %q into %qubits_iter[%i] : tensor<5x!qnet.qubit>
    affine.yield %qubits_updated : tensor<5x!qnet.qubit>
  }

  %t1_tensor = qnet.recv_floats {remote = @Bob} : tensor<1xf32>
  %t1 = tensor.extract %t1_tensor[%0] : tensor<1xf32>

  %m_init = tensor.empty() : tensor<5xi1>

  %m = affine.for %i = 0 to %5
      iter_args(%m_iter = %m_init) -> tensor<5xi1> {
    %q = tensor.extract %qubits[%i] : tensor<5x!qnet.qubit>
    %q2 = qnet.rot_x %q, %t1  : !qnet.qubit
    %m_i = qnet.measure %q2 : i1
    %m_updated = tensor.insert %m_i into %m_iter[%i] : tensor<5xi1>
    affine.yield %m_updated : tensor<5xi1>
  }
}