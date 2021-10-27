// RUN: hir-opt %s

%0 = "hir.alloc"() : () -> !hir.qubit
%1 = "hir.recv_cmsg"() : () -> !hir.cvalue
%2 = "hir.rot_x"(%0, %1) : (!hir.qubit, !hir.cvalue) -> !hir.qubit