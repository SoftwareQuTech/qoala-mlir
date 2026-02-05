// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s

// CHECK: META_START
// CHECK-NEXT: name: test_branching_int_constant_true
// CHECK-NEXT: parameters:
// CHECK-NEXT: csockets:
// CHECK-NEXT: epr_sockets:
// CHECK-NEXT: META_END
// CHECK-NEXT: ^b0 { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: %[[HOST_REG0:.*]] = assign_cval() : 0
// CHECK: ^b1 { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: %[[HOST_REG1:.*]] = assign_cval() : 3
// CHECK-NEXT: %[[HOST_REG2:.*]] = assign_cval() : 5
// CHECK-NEXT: %[[HOST_TRUE:.*]] = assign_cval() :
// CHECK-NEXT: jump() : b2
// CHECK: ^b2 { type = CL; predecessors = [b1]; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: send_cmsg(%[[HOST_REG0]], %[[HOST_REG1]])
// CHECK-NEXT: jump() : b4
// CHECK: ^b3 { type = CL; predecessors = [b2]; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: send_cmsg(%[[HOST_REG0]], %[[HOST_REG2]])
// CHECK-NEXT: jump() : b4
// CHECK: ^b4 { type = QL; predecessors = [b3]; dependencies = [b1]; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: tuple<%[[HOST_REG4:.*]]> = run_subroutine(tuple<%[[HOST_REG1]]>) : __qoala_wrapper0
// CHECK: ^b5 { type = CL; predecessors = []; dependencies = [b1, b4]; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: %[[HOST_REG5:.*]] = add_cval_c(%[[HOST_REG4]], %[[HOST_REG2]])
// CHECK-NEXT: return_result(%[[HOST_REG5]])

// CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params: p0
// CHECK-NEXT: returns: m0
// CHECK-NEXT: uses:
// CHECK-NEXT: keeps:
// CHECK-NEXT: request:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: load R[[ARG0_REG:.*]] @input[0]
// CHECK-NEXT: set C[[C_REG0:.*]] 10
// CHECK-NEXT: set C[[C_REG1:.*]]
// CHECK-NEXT: jmp 1
// CHECK-NEXT: add C[[C_REG2:.*]] R[[ARG0_REG]] C[[C_REG0]]
// CHECK-NEXT: jmp 3
// CHECK-NEXT: sub C[[C_REG3:.*]] R[[ARG0_REG]] C[[C_REG0]]
// CHECK-NEXT: jmp -3
// CHECK-NEXT: mul C[[C_REG4:.*]] R[[ARG0_REG]] C[[C_REG0]]
// CHECK-NEXT: store C[[C_REG4]] @output[0]
// CHECK-NEXT: NETQASM_END

module {
  qremote.remote @server
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> i1
  netqasm.local_routine @__qoala_wrapper0(%arg0: i32) -> i32 {
    %cstA = arith.constant 10 : i32
    %true = arith.constant 1 : i1
    cf.cond_br %true, ^bb1, ^bb2
  ^bb1:
    %1 = arith.addi %arg0, %cstA : i32
    cf.br ^bb3
  ^bb2:
    %2 = arith.subi %arg0, %cstA : i32
    cf.br ^bb1
  ^bb3:
    %3 = arith.muli %arg0, %cstA : i32
    netqasm.return %3 : i32
  }
  qoalahost.main_func @test_branching_int_constant_true() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.remote_id_ref  {classical = true, quantum = false, remote = @server}
    qoalahost.nop_term
  ^bb0:
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %cstA = arith.constant 3 : i32
    %cstB = arith.constant 5 : i32
    %true = arith.constant 1 : i1
    cf.cond_br %true, ^bb1, ^bb2
  ^bb1:
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = [], predecessors = ["block_1"], prev_comm = "", prev_ent = ""}
    qoalahost.send_int %cstA {remote = @server} : i32
    cf.br ^bb3
  ^bb2:
    qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = [], predecessors = ["block_2"], prev_comm = "", prev_ent = ""}
    qoalahost.send_int %cstB {remote = @server} : i32
    cf.br ^bb3
  ^bb3:
    qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = ["block_1"], predecessors = ["block_3"], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @__qoala_wrapper0(%cstA) : (i32) -> i32
  ^bb4:
     qoalahost.blk_meta  {block_id = "block_5", deadlines = {}, dependencies = ["block_1", "block_4"], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = arith.addi %0, %cstB : i32
    qoalahost.return %1 : i32
  }
}
