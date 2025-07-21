// RUN: qoala-opt %s --lower-qoala-mir-to-lir --qoala-opt-group-ent-reqs | FileCheck %s

module {
    qmem.remote @Bob

    qmem.func @test_group_ent() {
        %0 = qmem.qalloc : i32
        qmem.eprs %0 {remote = @Bob}

        %1 = qmem.qalloc : i32
        qmem.eprs %1 {remote = @Bob}

        qmem.return
    }
}
