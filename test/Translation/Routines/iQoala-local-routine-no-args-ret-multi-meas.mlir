// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META START
// CHECK-NEXT: name: test_call_local_routine
// CHECK-NEXT: parameters: Bob_id
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META END
// CHECK: ^b[[BLOCK0:.*]] { type = QL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: tuple<%[[HOST_REG0:.*]]; %[[HOST_REG1:.*]]; %[[HOST_REG2:.*]]> = run_subroutine() : __qoala_wrapper0

//CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params: {{[[:space:]]}}
// Since we are matching the newline char in the last check, we need to start matching
// on the same line!
// CHECK-SAME: returns: m0, m1, m2
// CHECK-NEXT: uses: [[QUBIT0:.*]], [[QUBIT1:.*]], [[QUBIT2:.*]]
// Since "meas" is considered a "qfree", this subroutine does not keep the qubit 0
// CHECK-NEXT: keeps: {{[[:space:]]}}
// Since we are matching the newline char in the last check, we need to start matching
// on the same line!
// CHECK-SAME: NETQASM_START
// CHECK-NEXT: set [[QUBIT_REG0:.*]] [[QUBIT0]]
// CHECK-NEXT: init [[QUBIT_REG0]]
// CHECK-NEXT: set [[QUBIT_REG1:.*]] [[QUBIT1]]
// CHECK-NEXT: init [[QUBIT_REG1]]
// CHECK-NEXT: set [[QUBIT_REG2:.*]] [[QUBIT2]]
// CHECK-NEXT: init [[QUBIT_REG2]]
// CHECK-NEXT: meas [[QUBIT_REG0]] [[M_REG0:.*]]
// CHECK-NEXT: meas [[QUBIT_REG1]] [[M_REG1:.*]]
// CHECK-NEXT: meas [[QUBIT_REG2]] [[M_REG2:.*]]
// CHECK-NEXT: store [[M_REG0]] @output[0]
// CHECK-NEXT: store [[M_REG1]] @output[1]
// CHECK-NEXT: store [[M_REG2]] @output[2]
// CHECK-NEXT: NETQASM_END

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @__qoala_wrapper0() -> (i1, i1, i1) {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    %1 = netqasm.qalloc  : i32
    netqasm.init %1
    %2 = netqasm.qalloc  : i32
    netqasm.init %2
    %3 = netqasm.measure %0 : i1
    %4 = netqasm.measure %1 : i1
    %5 = netqasm.measure %2 : i1
    netqasm.return %3, %4, %5 : i1, i1, i1
  }
  qoalahost.main_func @test_call_local_routine() {
    // Note: there is an implicit "^bb0" not-rendered block declaration here
    // so this "call" operation is the one and only operation of the
    // first block of the main function
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0, %1, %2 = qoalahost.call @__qoala_wrapper0() : () -> (i1, i1, i1)
    ^bb1:
      qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
      qoalahost.return
  }
}
