func.func @server() -> () {
^b0:
    %result1, %epr1 = "hir.entangle_keep"() { max_time = 20000 } : () -> (!hir.result, !hir.qubit)
    cf.cond_br %result1, ^b1, ^b0

^b1:
    %result2, %epr2 = "hir.entangle_keep"() { max_time = 1000 } : () -> (!hir.result, !hir.qubit)
    cf.cond_br %result2, ^b2, ^b0

^b2:
    %epr1a, %epr2a = "hir.two_gate"(%epr1, %epr2) : (!hir.qubit, !hir.qubit) -> (!hir.qubit, !hir.qubit)
    %delta1 = "hir.recv_int"() : () -> i32

    %epr2b = "hir.rotation_gate"(%epr2a, %delta1) : (!hir.qubit, i32) -> !hir.qubit
    %epr2c = "hir.gate"(%epr2b) : (!hir.qubit)-> !hir.qubit
    %m1 = "hir.measure"(%epr2c) : (!hir.qubit) -> i1

    "hir.send_bit"(%m1) : (i1) -> ()
    %delta2 = "hir.recv_int"() : () -> i32

    %epr1b = "hir.rotation_gate"(%epr1a, %delta2) : (!hir.qubit, i32) -> !hir.qubit
    %epr1c = "hir.gate"(%epr1b) : (!hir.qubit)-> !hir.qubit
    %m2 = "hir.measure"(%epr1c) : (!hir.qubit) -> i1

    "hir.send_bit"(%m2) : (i1) -> ()
    return
}