module {
  func.func @f() -> tensor<6xf64> {
    %0 = "toy.constant"() {idk = dense<[[1.000000e+00, 2.000000e+00, 3.000000e+00], [4.000000e+00, 5.000000e+00, 6.000000e+00]]> : tensor<2x3xf64>} : () -> tensor<2x3xf64>

    %1 = toy.reshape(%0 : tensor<2x3xf64>) to tensor<6xf64>
    %2 = toy.reshape(%1 : tensor<6xf64>) to tensor<6xf64>

    return %2 : tensor<6xf64>
  }
} 