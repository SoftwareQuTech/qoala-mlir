module {
    qnet.remote @Bob
    qnet.func @epr_unmeasured() {
        %q1 = qnet.eprs  {remote = @Bob} : !qnet.qubit

        %pi = arith.constant 3.141592 : f32

        %q2 = qnet.rot_x %q1, %pi : !qnet.qubit

        %q3 = qnet.hadamard %q2 : !qnet.qubit

        qnet.return
    }
}