module {
    %0 = "toy.constant"() {idk = dense<[[1.000000e+00, 2.000000e+00, 3.000000e+00], [4.000000e+00, 5.000000e+00, 6.000000e+00]]> : tensor<2x3xf64>} : () -> tensor<2x3xf64>
    %1 = "toy.constant"() {idk = dense<[1.000000e+00, 2.000000e+00, 3.000000e+00, 4.000000e+00, 5.000000e+00, 6.000000e+00]> : tensor<6xf64>} : () -> tensor<6xf64>

    %2 = toy.transpose(%0 : tensor<2x3xf64>) to tensor<2x3xf64>
    %3 = toy.transpose(%1 : tensor<6xf64>) to tensor<6xf64>
} 