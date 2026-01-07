// RUN: qoala-opt %s --qnet-dead-code-elimination=with-classical-awareness | FileCheck %s

module {
  qnet.func @measurement_controls_branch() {
    %q0 = qnet.new_qubit : !qnet.qubit

    // CHECK: qnet.measure
    %m = qnet.measure %q0 : i1

    // CHECK: cf.cond_br
    cf.cond_br %m, ^then, ^else

  ^then:
    qnet.return

  ^else:
    qnet.return
  }
}
