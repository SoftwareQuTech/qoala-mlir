// RUN: qoala-opt %s --qnet-dead-code-elimination=with-classical-awareness | FileCheck %s

module {
  qnet.remote @Bob

  qnet.func @measurement_sent_to_remote() {
    %q0 = qnet.new_qubit : !qnet.qubit

    // CHECK: qnet.measure
    %m = qnet.measure %q0 : i1

    %mi = arith.extui %m : i1 to i32

    // CHECK: qnet.send_int
    qnet.send_int %mi {remote = @Bob} : i32

    qnet.return
  }
}
