// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META START
// CHECK-NEXT: name: test_local_routine_args
// CHECK-NEXT: parameters: Bob
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META END
// CHECK-NEXT: b[[BLOCK0:.*]] { type = CL }
// CHECK-NEXT: %[[HOST_REG0:.*]] = assign_cval () : 3

//CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params: p0
// CHECK-NEXT: returns:
// CHECK-NEXT: uses:
// CHECK-NEXT: keeps:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set C[[C_REG0:.*]] 0
// CHECK-NEXT: load R[[R_REG0:.*]] @input[C[[C_REG0]]]
// CHECK-NEXT: set C[[C_REG1:.*]] 25
// CHECK-NEXT: NETQASM_END

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @__qoala_wrapper0(%arg0: i32) -> () {
    %cst = arith.constant 25 : i32
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    %1 = netqasm.measure %0 : i1
    netqasm.return
  }
  qoalahost.main_func @test_local_routine_args() {
    %cst = arith.constant 3 : i32
    %0 = qoalahost.call @__qoala_wrapper0(%cst) : (i32) -> i1
    qoalahost.return
  }
}
