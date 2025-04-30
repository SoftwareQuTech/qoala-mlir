// RUN: qoala-opt %s

module {
    qremote.remote @Bob

    netqasm.request_routine @req1() -> i32 {
      %vqubit = netqasm.qalloc : i32
      netqasm.eprs %vqubit {remote = @Bob}
      netqasm.return %vqubit : i32
    }

    netqasm.local_routine @subrt2(%vqubit: i32) {
      // For rotations, we need to specify the type of the arguments, otherwise they are
      // parsed as i64 in x86_64 which violates the constraints of the NetQASM operations
      netqasm.rot_x %vqubit (0 : ui32, 4 : ui32)
      netqasm.return
    }

    netqasm.local_routine @subrt3(%vqubit: i32) -> i1 {
      netqasm.rot_y %vqubit (0 : ui32, 4 : ui32)
      %m = netqasm.measure %vqubit : i1
      netqasm.return %m : i1
    }

    qoalahost.main_func @main() {
      qoalahost.blk_meta  {block_id = "block_0", predecessors = []}
      %zero = arith.constant 0 : index
      qoalahost.nop_term

    ^bb1:
      qoalahost.blk_meta  {block_id = "block_1", predecessors = []}
      %vqubit = qoalahost.call @req1() : () -> i32

    ^bb2:
      qoalahost.blk_meta  {block_id = "block_2", predecessors = []}
      %floats1 = qoalahost.recv_floats {remote = @Bob, length = 1 : i32} : tensor<1xf32>

    ^bb3:
      qoalahost.blk_meta  {block_id = "block_3", predecessors = ["block_0", "block_1", "block_2"]}
      %t1 = tensor.extract %floats1[%zero] : tensor<1xf32>
      // TODO: implement standard conversion code for float angle to int-tuple
      // %num1, %denom1 = func.call @conver_float_to_num_and_denom(%t1) : (f32) -> (i32, i32)

      qoalahost.call @subrt2(%vqubit) : (i32) -> ()

    ^bb4:
      qoalahost.blk_meta  {block_id = "block_4", predecessors = ["block_2"]}
      %floats2 = qoalahost.recv_floats {remote = @Bob, length = 1 : i32} : tensor<1xf32>

    ^bb5:
      qoalahost.blk_meta  {block_id = "block_5", predecessors = ["block_0", "block_1", "block_2"]}
      %t2 = tensor.extract %floats1[%zero] : tensor<1xf32>
      // %num2, %denom2 = func.call @conver_float_to_num_and_denom(%t1) : (f32) -> (i32, i32)

      %m = qoalahost.call @subrt3(%vqubit) : (i32) -> i1

    ^bb6:
      qoalahost.blk_meta  {block_id = "block_6", predecessors = []}
      qoalahost.return
    }
}