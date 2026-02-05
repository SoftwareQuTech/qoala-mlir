// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META_START
// CHECK-NEXT: name: quantum_alias_gates_program
// CHECK-NEXT: parameters: {{[[:space:]]}}
// CHECK-SAME: csockets: {{[[:space:]]}}
// CHECK-SAME: epr_sockets: {{[[:space:]]}}
// CHECK-SAME: META_END
// CHECK: ^b[[BLOCK0:.*]] { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: %[[VAL_1:.+]] = assign_cval() : 1
// CHECK-NEXT: %[[VAL_2:.+]] = assign_cval() : 2
// CHECK: ^b[[BLOCK1:.*]] { type = QL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: run_subroutine() : __qoala_wrapper0
// CHECK: ^b[[BLOCK2:.*]] { type = QL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: run_subroutine() : __qoala_wrapper1
// CHECK: ^b[[BLOCK3:.*]] { type = QL; predecessors = []; dependencies = [b0, b1]; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: run_subroutine(tuple<%[[VAL_2]]; %[[VAL_1]]>) : __qoala_wrapper2
// CHECK: ^b[[BLOCK4:.*]] { type = QL; predecessors = []; dependencies = [b1, b2]; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: run_subroutine() : __qoala_wrapper3
// CHECK: ^b[[BLOCK5:.*]] { type = QL; predecessors = []; dependencies = [b1]; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: %[[HOST_REG2:.*]] = run_subroutine() : __qoala_wrapper4
// CHECK: ^b[[BLOCK6:.*]] { type = QL; predecessors = []; dependencies = [b2]; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: %[[HOST_REG3:.*]] = run_subroutine() : __qoala_wrapper5

// CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params: {{[[:space:]]}}
// CHECK-SAME: returns: {{[[:space:]]}}
// CHECK-SAME: uses: [[QUBIT_0:.*]]
// CHECK-NEXT: keeps: [[QUBIT_0]]
// CHECK-NEXT: request:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set [[QREG0_0:.*]] [[QUBIT_0]]
// CHECK-NEXT: init [[QREG0_0]]
// CHECK-NEXT: NETQASM_END

// CHECK: SUBROUTINE __qoala_wrapper1
// CHECK-NEXT: params: {{[[:space:]]}}
// CHECK-SAME: returns: {{[[:space:]]}}
// CHECK-SAME: uses: [[QUBIT_1:.*]]
// CHECK-NEXT: keeps: [[QUBIT_1]]
// CHECK-NEXT: request:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set [[QREG0_1:.*]] [[QUBIT_1]]
// CHECK-NEXT: init [[QREG0_1]]
// CHECK-NEXT: NETQASM_END

// CHECK: SUBROUTINE __qoala_wrapper2
// CHECK-NEXT: params: p1, p2
// CHECK-NEXT: returns: {{[[:space:]]}}
// CHECK-SAME: uses: [[QUBIT_0]]
// CHECK-NEXT: keeps: [[QUBIT_0]]
// CHECK-NEXT: request:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set [[QREG0_2:.*]] [[QUBIT_0]]
// CHECK-NEXT: load [[VAL2:.*]] @input[1]
// CHECK-NEXT: load [[VAL1:.*]] @input[2]
// CHECK-NEXT: rot_x [[QREG0_2]] [[VAL2]] [[VAL1]]
// CHECK-NEXT: rot_y [[QREG0_2]] [[VAL2]] [[VAL1]]
// CHECK-NEXT: rot_z [[QREG0_2]] [[VAL2]] [[VAL1]]
// CHECK-NEXT: rot_z [[QREG0_2]] [[VAL1]] [[VAL1]]
// CHECK-NEXT: rot_z [[QREG0_2]] [[VAL1]] [[VAL2]]
// CHECK-NEXT: NETQASM_END

// CHECK: SUBROUTINE __qoala_wrapper3
// CHECK-NEXT: params: {{[[:space:]]}}
// CHECK-SAME: returns: {{[[:space:]]}}
// CHECK-SAME: uses: [[QUBIT_0]], [[QUBIT_1]]
// CHECK-NEXT: keeps: [[QUBIT_0]], [[QUBIT_1]]
// CHECK-NEXT: request:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set [[QREG0_3:.*]] [[QUBIT_0]]
// CHECK-NEXT: set [[QREG1_3:.*]] [[QUBIT_1]]
// CHECK-NEXT: cphase [[QREG0_3]] [[QREG1_3]]
// CHECK-NEXT: NETQASM_END

// CHECK: SUBROUTINE __qoala_wrapper4
// CHECK-NEXT: params: {{[[:space:]]}}
// CHECK-SAME: returns: m0
// CHECK-NEXT: uses: [[QUBIT_0]]
// CHECK-NEXT: keeps: {{[[:space:]]}}
// CHECK-SAME: request:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set [[QREG0_4:.*]] [[QUBIT_0]]
// CHECK-NEXT: meas [[QREG0_4]] [[MREG0_4:.*]]
// CHECK-NEXT: store [[MREG0_4]] @output[0]
// CHECK-NEXT: NETQASM_END

// CHECK: SUBROUTINE __qoala_wrapper5
// CHECK-NEXT: params: {{[[:space:]]}}
// CHECK-SAME: returns: m0
// CHECK-NEXT: uses: [[QUBIT_1]]
// CHECK-NEXT: keeps: {{[[:space:]]}}
// CHECK-SAME: request:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set [[QREG0_5:.*]] [[QUBIT_1]]
// CHECK-NEXT: meas [[QREG0_5]] [[MREG0_5:.*]]
// CHECK-NEXT: store [[MREG0_5]] @output[0]
// CHECK-NEXT: NETQASM_END

module {
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @__qoala_wrapper0() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.local_routine @__qoala_wrapper1() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.local_routine @__qoala_wrapper2(%arg0: i32, %two_arg: i32, %one_arg: i32) {
    netqasm.rot_x %arg0, %two_arg, %one_arg
    netqasm.rot_y %arg0, %two_arg, %one_arg
    netqasm.rot_z %arg0, %two_arg, %one_arg
    netqasm.rot_z %arg0, %one_arg, %one_arg
    netqasm.rot_z %arg0, %one_arg, %two_arg
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper3(%arg0: i32, %arg1: i32) {
    netqasm.cz %arg0, %arg1
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper4(%arg0: i32) -> i1 {
    %0 = netqasm.measure %arg0 : i1
    netqasm.return %0 : i1
  }
  netqasm.local_routine @__qoala_wrapper5(%arg0: i32) -> i1 {
    %0 = netqasm.measure %arg0 : i1
    netqasm.return %0 : i1
  }
  qoalahost.main_func @quantum_alias_gates_program() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %c1_i32 = arith.constant 1 : i32
    %c2_i32 = arith.constant 2 : i32
    qoalahost.nop_term
  ^bb0:
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @__qoala_wrapper0() : () -> i32
  ^bb1:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = qoalahost.call @__qoala_wrapper1() : () -> i32
  ^bb2:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = ["block_0", "block_1"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper2(%0, %c2_i32, %c1_i32) : (i32, i32, i32) -> ()
  ^bb3:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = ["block_1", "block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper3(%0, %1) : (i32, i32) -> ()
  ^bb4:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_5", deadlines = {}, dependencies = ["block_1"], predecessors = [], prev_comm = "", prev_ent = ""}
    %2 = qoalahost.call @__qoala_wrapper4(%0) : (i32) -> i1
  ^bb5:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_6", deadlines = {}, dependencies = ["block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
    %3 = qoalahost.call @__qoala_wrapper5(%1) : (i32) -> i1
  ^bb6:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_7", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.return
  }
}