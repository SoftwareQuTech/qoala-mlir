func @client(%alpha: i32, %beta: i32, %theta1: i32, %theta2: i32) -> (i1, i1, i1) {
    %epr1 = "hir.entangle_keep"() { max_time = 1000 : i32 } : () -> (!hir.result, !hir.qubit, !hir.array)
    %epr1a = "hir.rotation_gate"(%epr1, %theta2) : (!hir.qubit, i32) -> !hir.qubit
    %epr1b = "hir.gate"(%epr1a) : (!hir.qubit) -> !hir.qubit
    %p2 = "hir.meas"(%epr1b) : (!hir.qubit) -> i1

    %epr2 = "hir.entangle_keep"() { max_time = 1000 : i32 } : () -> (!hir.result, !hir.qubit, !hir.array)
    %epr2a = "hir.rotation_gate"(%epr2, %theta1) : (!hir.qubit, i32) -> !hir.qubit
    %epr2b = "hir.gate"(%epr2a) : (!hir.qubit) -> !hir.qubit
    %p1 = "hir.meas"(%epr2b) : (!hir.qubit) -> i1

    %delta1 = "hir.sub"(%alpha, %theta1) : (i32, i32) -> i32
    %p1a = "hir.bit_to_int"(%p1) : (i1) -> i32
    %p1b = "hir.multiply_const"(%p1a) { const = 16 } : (i32) -> i32
    %delta1a = "hir.add"(%delta1, %p1b) : (i32, i32) -> i32

    "hir.send_int"(%delta1a) : (i32) -> ()
    %m1 = "hir.recv_bit"() { max_time = 25000: i32 } : () -> (!hir.result, i1)

    %delta2 = "hir.cond_multiply_const"(%beta, %m1) { const = -1: i32 } : (i32, i1) -> i32
    %delta2a = "hir.sub"(%delta2, %theta2) : (i32, i32) -> i32
    %p2a = "hir.bit_to_int"(%p2) : (i1) -> i32
    %p2b = "hir.multiply_const"(%p2a) { const = 16 } : (i32) -> i32
    %delta2b = "hir.add"(%delta2a, %p2b) : (i32, i32) -> i32

    "hir.send_int"(%delta2b) : (i32) -> ()
    %m2 = "hir.recv_bit"() { max_time = 25000: i32 } : () -> (!hir.result, i1)

    return (%p1, %p2, %m2)
}