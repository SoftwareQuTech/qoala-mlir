// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META START
// CHECK-NEXT: name: test_host_ret_one_val
// CHECK-NEXT: parameters: Bob_id
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META END
// CHECK-NEXT: b[[BLOCK0:.*]] { type = CL }
// CHECK-NEXT: %[[HOST_REG0:.*]] = assign_cval() : 3
// CHECK: b[[BLOCK1:.*]] { type = QL }
// CHECK-NEXT: %[[HOST_REG1:.*]] = run_subroutine() : __qoala_wrapper0
// CHECK: b[[BLOCK2:.*]] { type = CL }
// CHECK-NEXT: %[[HOST_REG2:.*]] = add_cval_c(%[[HOST_REG1]], %[[HOST_REG0]])
// CHECK-NEXT: return_value(%[[HOST_REG2]])

//CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params: {{[[:space:]]}}
// CHECK-SAME: returns: m0
// CHECK-NEXT: uses: {{[[:space:]]}}
// CHECK-SAME: keeps: {{[[:space:]]}}
// CHECK-SAME: NETQASM_START
// CHECK-NEXT: set C[[C_REG0:.*]] 1
// CHECK-NEXT: store C[[C_REG0]] @output[0]
// CHECK-NEXT: NETQASM_END

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @__qoala_wrapper0() -> i32 {
    %cst = arith.constant 1 : i32
    netqasm.return %cst : i32
  }
  qoalahost.main_func @test_host_ret_one_val() -> i32 {
    %cst = arith.constant 3 : i32
    qoalahost.nop_term
  ^bb1:
    %0 = qoalahost.call @__qoala_wrapper0() : () -> i32
  ^bb2:
    %1 = arith.addi %0, %cst : i32
    qoalahost.return %1 : i32
  }
}
