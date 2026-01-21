// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s

// CHECK: META_START
// CHECK-NEXT: name: example
// CHECK-NEXT: parameters:
// CHECK-NEXT: csockets:
// CHECK-NEXT: epr_sockets:
// CHECK-NEXT: META_END

// CHECK: ^[[BLOCK0:.*]] { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: %[[HOST_REG0:.*]] = assign_cval() : 0
// CHECK: ^[[BLOCK1:.*]] { type = QL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: tuple<%[[HOST_REG1:.*]]> = run_subroutine() : __qoala_wrapper0

// The important bit: extui must not appear in output, and send uses the returned value.
// CHECK: ^[[BLOCK2:.*]] { type = CL; predecessors = []; dependencies = [[[BLOCK0]], [[BLOCK1]]]; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: send_cmsg(%[[HOST_REG0]], %[[HOST_REG1]])

// Ensure no extui artifact survives in text output.
// CHECK-NOT: extui

module {
  qremote.remote @Alice
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @__qoala_wrapper0() -> i1 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    %1 = netqasm.measure %0 : i1
    netqasm.return %1 : i1
  }
  qoalahost.main_func @example() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.remote_id_ref  {classical = true, quantum = false, remote = @Alice}
    qoalahost.nop_term
  ^bb1:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @__qoala_wrapper0() : () -> i1
  ^bb2:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = ["block_0", "block_1"], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = arith.extui %0 : i1 to i32
    qoalahost.send_int %1 {remote = @Alice} : i32
    qoalahost.return
  }
}
