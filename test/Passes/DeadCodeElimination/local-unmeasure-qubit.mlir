module {
    qnet.func @local_unmeasured_qubit() {
        %q1 = qnet.new_qubit : !qnet.qubit

        %pi = arith.constant 3.141592 : f32

        %q2 = qnet.rot_x %q1, %pi : !qnet.qubit

        %q3 = qnet.hadamard %q2 : !qnet.qubit

        qnet.return
    }
}