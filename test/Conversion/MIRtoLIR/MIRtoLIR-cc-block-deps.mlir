// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

module {
    qmem.remote @Bob
    qmem.func @test_add_cc_block_deps() {
        // CHECK: qoalahost.blk_meta  {block_id = "block_0", predecessors = []}
        %csti = arith.constant 2 : i32
        %cstf = arith.constant 2.120000e+01 : f32

        %to_sendi = tensor.from_elements %csti : tensor<1xi32>
        qmem.send_ints %to_sendi {remote = @Bob} : tensor<1xi32>

        // CHECK: qoalahost.blk_meta  {block_id = "block_1", predecessors = ["block_0"]}
        %to_recvf = qmem.recv_floats {length = 2 : i32, remote = @Bob} : tensor<2xf32>

        // CHECK: qoalahost.blk_meta  {block_id = "block_2", predecessors = ["block_1"]}
        %to_recvi = qmem.recv_ints {length = 2 : i32, remote = @Bob} : tensor<2xi32>

        // CHECK: qoalahost.blk_meta  {block_id = "block_3", predecessors = ["block_0", "block_2"]}
        %to_sendf = tensor.from_elements %cstf : tensor<1xf32>
        qmem.send_floats %to_sendf {remote = @Bob} : tensor<1xf32>

        qmem.return
    }
}
