// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META START
// CHECK-NEXT: name: test_local_routine_args
// CHECK-NEXT: parameters: Bob
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META END
// CHECK-NEXT: b[[BLOCK0:.*]] { type = CL }
// CHECK-NEXT: %[[HOST_REG0:.*]] = assign_cval () : 3
// CHECK-NEXT: %[[HOST_REG1:.*]] = assign_cval () : 5
// CHECK-NEXT: %[[HOST_REG2:.*]] = assign_cval () : 2

//CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params: p0, p1, p2
// CHECK-NEXT: returns:
// CHECK-NEXT: uses:
// CHECK-NEXT: keeps:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set C[[C_REG0:.*]] 0
// CHECK-NEXT: load R[[R_REG0:.*]] @input[C[[C_REG0]]]
// CHECK-NEXT: set C[[C_REG1:.*]] 1
// CHECK-NEXT: load R[[R_REG1:.*]] @input[C[[C_REG1]]]
// CHECK-NEXT: set C[[C_REG2:.*]] 2
// CHECK-NEXT: load R[[R_REG2:.*]] @input[C[[C_REG2]]]
// CHECK-NEXT: set C[[C_REG3:.*]] 25
// CHECK-NEXT: NETQASM_END

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @__qoala_wrapper0(%arg0: i32, %arg1: i32, %arg2: i32) -> () {
    %cst = arith.constant 25 : i32
    netqasm.return
  }
  qoalahost.main_func @test_local_routine_args() {
    %cstA = arith.constant 3 : i32
    %cstB = arith.constant 5 : i32
    %cstC = arith.constant 2 : i32
    qoalahost.nop_term
  ^bb1:
    qoalahost.call @__qoala_wrapper0(%cstA, %cstB, %cstC) : (i32, i32, i32) -> ()
  ^bb2:
    qoalahost.return
  }
}
