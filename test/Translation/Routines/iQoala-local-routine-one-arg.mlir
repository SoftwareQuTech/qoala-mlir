// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META START
// CHECK-NEXT: name: test_local_routine_args
// CHECK-NEXT: parameters: Bob_id
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META END
// CHECK-NEXT: b[[BLOCK0:.*]] { type = CL }
// CHECK-NEXT: %[[HOST_REG0:.*]] = assign_cval() : 3
// CHECK: b[[BLOCK1:.*]] { type = CL }
// CHECK-NEXT: run_subroutine(tuple<%[[HOST_REG0]]>) : __qoala_wrapper0

//CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params: p0
// CHECK-NEXT: returns: {{[[:space:]]}}
// CHECK-SAME: uses: {{[[:space:]]}}
// CHECK-SAME: keeps: {{[[:space:]]}}
// CHECK-SAME: NETQASM_START
// CHECK-NEXT: load R[[R_REG0:.*]] @input[0]
// CHECK-NEXT: set C[[C_REG1:.*]] 25
// CHECK-NEXT: NETQASM_END

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @__qoala_wrapper0(%arg0: i32) -> () {
    %cst = arith.constant 25 : i32
    netqasm.return
  }
  qoalahost.main_func @test_local_routine_args() {
    %cst = arith.constant 3 : i32
    qoalahost.nop_term
  ^bb1:
    qoalahost.call @__qoala_wrapper0(%cst) : (i32) -> ()
  ^bb2:
    qoalahost.return
  }
}
