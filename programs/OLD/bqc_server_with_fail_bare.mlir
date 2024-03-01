"builtin.module"() ({
    "func.func"() ({
        %q0, %succ0 = "netqasm.entangle_keep_with_fail"() : () -> (!netqasm.equbit, i1)
        "cf.cond_br"(%succ0) [^q0_succ, ^q0_fail] 
            <{operandSegmentSizes = array<i32: 1, 0, 0>}> : (i1) -> ()
    ^q0_succ():
        %q1, %succ1 = "netqasm.entangle_keep_with_fail"() : () -> (!netqasm.equbit, i1)
        %succ = "arith.constant"() {value = 0 : i1} : () -> (i1)
        "func.return"(%q0, %q1, %succ) : (!netqasm.equbit, !netqasm.equbit, i1) -> ()
    ^q0_fail():
        %fail = "arith.constant"() {value = 0 : i1} : () -> (i1)
        "func.return"(%q0, %q0, %fail) : (!netqasm.equbit, !netqasm.equbit, i1) -> ()
    }) {
        sym_name = "rsp",
        function_type = () -> (!netqasm.equbit, !netqasm.equbit, i1)
    } : () -> ()

    %e0, %e1, %esucc = "func.call"() {callee = @rsp} : () -> (!netqasm.equbit, !netqasm.equbit, i1)
}) : () -> ()