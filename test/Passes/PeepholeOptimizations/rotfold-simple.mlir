// RUN: qoala-opt %s --qnet-peephole-optimizations=rotation-folding | FileCheck %s

module {
    qnet.func @rotation_folding_simple() {

        %q1 = qnet.new_qubit : !qnet.qubit

        %cst = arith.constant 3.14159274 : f32

        %q2 = qnet.rot_x %q1, %cst : !qnet.qubit
        %q3 = qnet.rot_x %q2, %cst : !qnet.qubit

        %q4 = qnet.rot_y %q3, %cst : !qnet.qubit
        %q5 = qnet.rot_y %q4, %cst : !qnet.qubit

        %q6 = qnet.rot_z %q5, %cst : !qnet.qubit
        %q7 = qnet.rot_z %q6, %cst : !qnet.qubit

        %q8 = qnet.new_qubit : !qnet.qubit

        %q9, %q10 = qnet.crot_x %q8, %q7, %cst : !qnet.qubit, !qnet.qubit
        %q11, %q12 = qnet.crot_x %q9, %q10, %cst : !qnet.qubit, !qnet.qubit

        qnet.return
    }
}