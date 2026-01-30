module {
  qnet.remote @Bob
  qnet.func @recapture_qubit_val() {
    %0 = qnet.eprs  {remote = @Bob} : !qnet.qubit
    %1 = qnet.recv_int  {remote = @Bob} : i32
    %2 = qnet.recv_int  {remote = @Bob} : i32
    %c1_i32 = arith.constant 1 : i32
    %3 = arith.cmpi eq, %1, %c1_i32 : i32
    %4 = scf.if %3 -> (!qnet.qubit) {
      %8 = qnet.z %0 : !qnet.qubit
      scf.yield %8 : !qnet.qubit
    } else {
      scf.yield %0 : !qnet.qubit
    }
    %c1_i32_0 = arith.constant 1 : i32
    %5 = arith.cmpi eq, %2, %c1_i32_0 : i32
    %6 = scf.if %5 -> (!qnet.qubit) {
      %8 = qnet.x %4 : !qnet.qubit
      scf.yield %8 : !qnet.qubit
    } else {
      scf.yield %4 : !qnet.qubit
    }
    %7 = qnet.measure %6 : i1
    qnet.return
  }
}
