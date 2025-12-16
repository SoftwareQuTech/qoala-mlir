// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META_START
// CHECK-NEXT: name: test_remote_quantum_program
// CHECK-NEXT: parameters: Bob_id
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META_END
// CHECK-NEXT: ^b[[BLOCK0:.*]] { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: %[[VAL_5:.*]] = assign_cval() : 5
// CHECK-NEXT: %[[VAL_0:.*]] = assign_cval() : 0
// CHECK-NEXT: ^b[[BLOCK1:.*]] { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: %[[CSOCK_A:.*]] = assign_cval() : 0
// CHECK-NEXT: %[[RECV_INT_A:.*]] = recv_cmsg([[CSOCK_A]])
// CHECK-NEXT: ^b[[BLOCK2:.*]] { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: %[[CSOCK_B:.*]] = assign_cval() : 0
// CHECK-NEXT: %[[RECV_INT_B:.*]] = recv_cmsg([[CSOCK_B]])
// CHECK-NEXT: ^b[[BLOCK3:.*]] { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: %[[CSOCK_C:.*]] = assign_cval() : 0
// CHECK-NEXT: send_cmsg([[CSOCK_C]], %[[VAL_0]])
// CHECK-NEXT: %[[CSOCK_D:.*]] = assign_cval() : 0
// CHECK-NEXT: send_cmsg([[CSOCK_D]], %[[VAL_5]])

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  qoalahost.main_func @test_remote_quantum_program() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %c5_i32 = arith.constant 5 : i32
    %c0_i32 = arith.constant 0 : i32
    qoalahost.nop_term
  ^bb1:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.recv_int  {remote = @Bob} : i32
  ^bb2:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "block_1", prev_ent = ""}
    %1 = qoalahost.recv_int  {remote = @Bob} : i32
  ^bb3:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "block_2", prev_ent = ""}
    qoalahost.send_int %c0_i32 {remote = @Bob} : i32
    qoalahost.send_int %c5_i32 {remote = @Bob} : i32
    qoalahost.return
  }
}
