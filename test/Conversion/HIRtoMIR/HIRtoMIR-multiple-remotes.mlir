// RUN: qoala-opt %s --lower-qoala-hir-to-mir | FileCheck %s

// CHECK: module
module {
    //CHECK: qmem.remote @[[REMOTEBOB:.*]]
    qnet.remote @Bob
    //CHECK: qmem.remote @[[REMOTECHARLIE:.*]]
    qnet.remote @Charlie

    qnet.func @do_nothing() {
        // CHECK: qmem.eprs %[[QBIT0:.*]] {remote = @[[REMOTEBOB]]}
        %0 = qnet.eprs  {remote = @Bob} : !qnet.qubit
        // CHECK: qmem.eprs %[[QBIT1:.*]] {remote = @[[REMOTECHARLIE]]}
        %1 = qnet.eprs  {remote = @Charlie} : !qnet.qubit
        qnet.return
    }
}
