// RUN: qoala-opt %s --qnet-dead-code-elimination=with-classical-awareness | FileCheck %s

module {
  qnet.func @unused_measurement_is_dead() {
    // CHECK-NOT: qnet.new_qubit
    %q0 = qnet.new_qubit : !qnet.qubit

    %pi = arith.constant 3.141592 : f32

    // CHECK-NOT: qnet.rot_x
    %q1 = qnet.rot_x %q0, %pi : !qnet.qubit

    // CHECK-NOT: qnet.measure
    %m = qnet.measure %q1 : i1

    qnet.return
  }
}
