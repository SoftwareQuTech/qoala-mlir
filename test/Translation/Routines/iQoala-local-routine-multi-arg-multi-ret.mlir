// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META START
// CHECK-NEXT: name: test_local_routine_ret_one_val
// CHECK-NEXT: parameters: Bob_id
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META END
// CHECK-NEXT: b[[BLOCK0:.*]] { type = CL }
// CHECK-NEXT: %[[HOST_REG0:.*]] = assign_cval() : 3
// CHECK-NEXT: %[[HOST_REG1:.*]] = assign_cval() : 7
// CHECK: b1 { type = CL }
// CHECK-NEXT: tuple<%[[HOST_REG2:.*]]; %[[HOST_REG3:.*]]; %[[HOST_REG4:.*]]> = run_subroutine(tuple<%[[HOST_REG0]]; %[[HOST_REG1]]>) : __qoala_wrapper0

//CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params: p0, p1
// CHECK-NEXT: returns: m0, m1, m2
// CHECK-NEXT: uses: {{[[:space:]]}}
// CHECK-SAME: keeps: {{[[:space:]]}}
// CHECK-SAME: NETQASM_START
// CHECK-NEXT: load R[[R_REG0:.*]] @input[0]
// CHECK-NEXT: load R[[R_REG1:.*]] @input[1]
// CHECK-NEXT: set C[[C_REG2:.*]] 11
// CHECK-NEXT: add C[[C_REG3:.*]] R[[R_REG0]] C[[C_REG2]]
// CHECK-NEXT: sub C[[C_REG4:.*]] R[[R_REG1]] C[[C_REG2]]
// CHECK-NEXT: mul C[[C_REG5:.*]] R[[R_REG1]] C[[C_REG2]]
// CHECK-NEXT: store C[[C_REG3]] @output[0]
// CHECK-NEXT: store C[[C_REG4]] @output[1]
// CHECK-NEXT: store C[[C_REG5]] @output[2]
// CHECK-NEXT: NETQASM_END

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @__qoala_wrapper0(%arg0: i32, %arg1: i32) -> (i32, i32, i32) {
    %cst = arith.constant 11 : i32
    %retA = arith.addi %arg0, %cst : i32
    %retB = arith.subi %arg1, %cst : i32
    %retC = arith.muli %arg1, %cst : i32
    netqasm.return %retA, %retB, %retC : i32, i32, i32
  }
  qoalahost.main_func @test_local_routine_ret_one_val() {
    %cstA = arith.constant 3 : i32
    %cstB = arith.constant 7 : i32
    qoalahost.nop_term
  ^bb1:
    %0, %1, %2 = qoalahost.call @__qoala_wrapper0(%cstA, %cstB) : (i32, i32) -> (i32, i32, i32)
  ^bb2:
    qoalahost.return
  }
}
