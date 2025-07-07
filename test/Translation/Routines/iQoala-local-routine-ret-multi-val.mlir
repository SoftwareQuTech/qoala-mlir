// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META START
// CHECK-NEXT: name: test_local_routine_ret_one_val
// CHECK-NEXT: parameters: Bob_id
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META END
// CHECK-NEXT: ^b[[BLOCK0:.*]] { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = }
// CHECK-NEXT: %[[HOST_REG0:.*]] = assign_cval() : 3
// CHECK: ^b[[BLOCK1:.*]] { type = QL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = }
// CHECK-NEXT: tuple<%[[HOST_REG0:.*]]; %[[HOST_REG1:.*]]; %[[HOST_REG2:.*]]> = run_subroutine() : __qoala_wrapper0

//CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params: {{[[:space:]]}}
// CHECK-SAME: returns: m0, m1, m2
// CHECK-NEXT: uses: {{[[:space:]]}}
// CHECK-SAME: keeps: {{[[:space:]]}}
// CHECK-SAME: NETQASM_START
// CHECK-NEXT: set C[[C_REG0:.*]] 1
// CHECK-NEXT: set C[[C_REG1:.*]] 2
// CHECK-NEXT: set C[[C_REG2:.*]] 3
// CHECK-NEXT: store C[[C_REG0]] @output[0]
// CHECK-NEXT: store C[[C_REG1]] @output[1]
// CHECK-NEXT: store C[[C_REG2]] @output[2]
// CHECK-NEXT: NETQASM_END

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @__qoala_wrapper0() -> (i32, i32, i32) {
    %cstA = arith.constant 1 : i32
    %cstB = arith.constant 2 : i32
    %cstC = arith.constant 3 : i32
    netqasm.return %cstA, %cstB, %cstC : i32, i32, i32
  }
  qoalahost.main_func @test_local_routine_ret_one_val() {
    qoalahost.blk_meta  {block_id = "block_0", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %cst = arith.constant 3 : i32
    qoalahost.nop_term
  ^bb1:
    qoalahost.blk_meta  {block_id = "block_1", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0, %1, %2 = qoalahost.call @__qoala_wrapper0() : () -> (i32, i32, i32)
  ^bb2:
    qoalahost.blk_meta  {block_id = "block_2", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.return
  }
}
