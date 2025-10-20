module {
    qnet.func @unmeasured_locally_entangled_qubit() {
        %q1 = qnet.new_qubit : !qnet.qubit
        %q2 = qnet.new_qubit : !qnet.qubit

        %q3, %q4 = qnet.cnot %q1, %q2 : !qnet.qubit, !qnet.qubit

        %pi = arith.constant 3.141592 : f32

        %q5 = qnet.rot_x %q4, %pi : !qnet.qubit

        %m = qnet.measure %q3 : i1

        qnet.return
    }
}