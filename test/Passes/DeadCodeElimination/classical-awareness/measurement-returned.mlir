// RUN: qoala-opt %s --qnet-dead-code-elimination=with-classical-awareness | FileCheck %s

module {
  qnet.func @measurement_returned() -> i1 {
    // CHECK: qnet.new_qubit
    %q0 = qnet.new_qubit : !qnet.qubit

    // CHECK: qnet.measure
    %m = qnet.measure %q0 : i1

    // CHECK: qnet.return
    qnet.return %m : i1
  }
}
