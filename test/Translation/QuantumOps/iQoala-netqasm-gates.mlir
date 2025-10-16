// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META_START
// CHECK-NEXT: name: test_local_routine_gates
// CHECK-NEXT: parameters: Bob_id
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META_END
// CHECK: ^b[[BLOCK0:.*]] { type = QL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: tuple<%[[RES_0:.*]]; %[[RES_1:.*]]> = run_subroutine() : __qoala_wrapper0
// There is only one block because ^bb1 only has a `qoalahost.return` which does not return anything.
// Thus, the block gets deleted

//CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params: {{[[:space:]]}}
// CHECK-SAME: returns: m0, m1
// CHECK-NEXT: uses: [[PHY_QUBIT0:.*]], [[PHY_QUBIT1:.*]]
// CHECK-NEXT: keeps: {{[[:space:]]}}
// CHECK-SAME: NETQASM_START
// CHECK-NEXT: set [[QBIT0:.*]] [[PHY_QUBIT0]]
// CHECK-NEXT: init [[QBIT0]]
// CHECK-NEXT: set [[QBIT1:.*]] [[PHY_QUBIT1]]
// CHECK-NEXT: init [[QBIT1]]
// CHECK-NEXT: h [[QBIT0]]
// CHECK-NEXT: cnot [[QBIT0]] [[QBIT1]]
// CHECK-NEXT: cphase [[QBIT0]] [[QBIT1]]
// CHECK-NEXT: meas [[QBIT0]] [[M_REG0:.*]]
// CHECK-NEXT: meas [[QBIT1]] [[M_REG1:.*]]
// CHECK-NEXT: store [[M_REG0]] @output[0]
// CHECK-NEXT: store [[M_REG1]] @output[1]
// CHECK-NEXT: NETQASM_END

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @__qoala_wrapper0() -> (i1, i1) {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    %1 = netqasm.qalloc  : i32
    netqasm.init %1
    netqasm.hadamard %0
    netqasm.cnot %0, %1
    netqasm.cz %0, %1
    %2 = netqasm.measure %0 : i1
    %3 = netqasm.measure %1 : i1
    netqasm.return %2, %3 : i1, i1
  }
  qoalahost.main_func @test_local_routine_gates() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0, %1 = qoalahost.call @__qoala_wrapper0() : () -> (i1, i1)
  ^bb1:
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.return
  }
}