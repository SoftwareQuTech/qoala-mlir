// RUN: qoala-opt %s --qoalahost-add-block-deps | FileCheck %s

module {
    qoalahost.main_func @test_add_block_deps_idempotence() {
// CHECK: qoalahost.blk_meta {block_id = "block_0", predecessors = []}
// CHECK-NEXT: qoalahost.return
        qoalahost.blk_meta {block_id = "block_0", predecessors = []}
        qoalahost.return
    }
}
