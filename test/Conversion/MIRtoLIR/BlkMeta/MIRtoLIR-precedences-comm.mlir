// RUN: qoala-opt %s --lower-qoala-mir-to-lir=disable-unfold-comm-ops | FileCheck %s

module {
    qmem.remote @Bob
    qmem.func @test_add_cc_block_deps() {
        // CHECK: qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        // CHECK: qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
        %csti = arith.constant 2 : i32
        %cstf = arith.constant 2.120000e+01 : f32

        %to_sendi = tensor.from_elements %csti : tensor<1xi32>
        qmem.send_ints %to_sendi {remote = @Bob} : tensor<1xi32>

        // CHECK: qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "block_1", prev_ent = ""}
        %to_recvf = qmem.recv_floats {length = 2 : i32, remote = @Bob} : tensor<2xf32>

        // CHECK: qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "block_2", prev_ent = ""}
        %to_recvi = qmem.recv_ints {length = 2 : i32, remote = @Bob} : tensor<2xi32>

        // CHECK: qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = ["block_1"], predecessors = [], prev_comm = "block_3", prev_ent = ""}
        %to_sendf = tensor.from_elements %cstf : tensor<1xf32>
        qmem.send_floats %to_sendf {remote = @Bob} : tensor<1xf32>

        qmem.return
    }
}
