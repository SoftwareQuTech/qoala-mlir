// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// TODO - Use this test case to make test for other branching conditions (neq, blt, bgt, etc)
// CHECK: META START
// CHECK-NEXT: name: test_branching
// CHECK-NEXT: parameters: Bob
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META END
// CHECK-NEXT: b[[BLOCK0:.*]] { type = CL }
// CHECK-NEXT: %[[HOST_REG0:.*]] = assign_cval () : 3
// CHECK-NEXT: %[[HOST_REG1:.*]] = assign_cval () : 2
// TODO - Find a way to make FileCheck to match a "forward" label
// CHECK-NEXT: beq (%[[HOST_REG0]], %[[HOST_REG1]]) : b1
// CHECK-NEXT: jump () : b2
// CHECK: b[[BLOCK1:.*]] { type = CL }
// CHECK-NEXT: jump () : b3
// CHECK: b[[BLOCK2:.*]] { type = CL }
// CHECK-NEXT: jump () : b3
// CHECK: b[[BLOCK3:.*]] { type = CL }
// CHECK-NEXT: %[[HOST_REG2:.*]] = add_cval_c (%[[HOST_REG0:.*]], %[[HOST_REG1:.*]])

//CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params: p0
// CHECK-NEXT: returns: m0
// CHECK-NEXT: uses:
// CHECK-NEXT: keeps:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set C[[C_REG0:.*]] 0
// CHECK-NEXT: load R[[C_REG0:.*]] @input[C[[C_REG0]]]
// CHECK-NEXT: set C[[C_REG1:.*]] 10
// CHECK-NEXT: set C[[C_REG2:.*]] 5
// CHECK-NEXT: store C[[C_REG2]] @output[0]
// CHECK-NEXT: NETQASM_END

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> i1
  netqasm.local_routine @__qoala_wrapper0(%arg0: i32) -> i32 {
    %cstA = arith.constant 10 : i32
    %cstB = arith.constant 5 : i32
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    %jump_loc = arith.cmpi eq, %cstA, %arg0 : i32
    cf.cond_br %jump_loc, ^bb0, ^bb1
  ^bb0:
    netqasm.rot_x %0 (3 : ui32, 2 : ui32)
    cf.br ^bb2
  ^bb1:
    netqasm.rot_y %0 (1 : ui32, 2 : ui32)
    cf.br ^bb2
  ^bb2:
    %1 = netqasm.measure %0 : i1
    netqasm.return %cstB : i32
  }
  qoalahost.main_func @test_branching() {
    %cstA = arith.constant 3 : i32
    %cstB = arith.constant 2 : i32
    %jump = arith.cmpi eq, %cstA, %cstB : i32
    cf.cond_br %jump, ^bb0, ^bb1
  ^bb0:
    cf.br ^bb2
  ^bb1:
    cf.br ^bb2
  ^bb2:
    %0 = qoalahost.call @__qoala_wrapper0(%cstA) : (i32) -> i32
    %1 = arith.addi %cstA, %cstB : i32
    qoalahost.return
  }
}
