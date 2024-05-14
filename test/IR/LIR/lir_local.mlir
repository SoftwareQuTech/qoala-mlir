// RUN: qoala-opt %s

module {
    qremote.remote @Bob

    netqasm.local_routine @subrt1() -> i32 {
      %vqubit = netqasm.qalloc : i32
      netqasm.init %vqubit
      netqasm.return %vqubit : i32
    }

    netqasm.local_routine @subrt2(%vqubit: i32, %angle_num: i32, %angle_denom: i32) {
      netqasm.rot_x %vqubit, %angle_num, %angle_denom
      netqasm.return
    }

    netqasm.local_routine @subrt3(%vqubit: i32, %angle_num: i32, %angle_denom: i32) -> i1 {
      netqasm.rot_y %vqubit, %angle_num, %angle_denom
      %m = netqasm.measure %vqubit : i1
      netqasm.return %m : i1
    }

    qoalahost.main_func @main() {
      %zero = arith.constant 0 : index
      %vqubit = qoalahost.call @subrt1() : () -> i32

      %floats1 = qoalahost.recv_floats {remote = @Bob, length = 1 : i32} : tensor<1xf32>
      %t1 = tensor.extract %floats1[%zero] : tensor<1xf32>
      // TODO: implement standard conversion code for float angle to int-tuple
      // %num1, %denom1 = qoalahost.call @conver_float_to_num_and_denom(%t1) : (f32) -> (i32, i32)
      %num1 = arith.constant 0 : i32
      %denom1 = arith.constant 4 : i32
      qoalahost.call @subrt2(%vqubit, %num1, %denom1) : (i32, i32, i32) -> ()

      %floats2 = qoalahost.recv_floats {remote = @Bob, length = 1 : i32} : tensor<1xf32>
      %t2 = tensor.extract %floats1[%zero] : tensor<1xf32>
      // %num2, %denom2 = qoalahost.call @conver_float_to_num_and_denom(%t1) : (f32) -> (i32, i32)
      %num2 = arith.constant 0 : i32
      %denom2 = arith.constant 4 : i32

      %m = qoalahost.call @subrt3(%vqubit, %num2, %denom2) : (i32, i32, i32) -> i1
      qoalahost.return
    }
}