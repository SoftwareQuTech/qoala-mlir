module {
    %0 = "iqoala.host_const"() {value = 3: i32} : () -> i32
    %1 = "iqoala.constant"() {idk = dense<[1.000000e+00, 2.000000e+00, 3.000000e+00, 4.000000e+00, 5.000000e+00, 6.000000e+00]> : tensor<6xf64>} : () -> tensor<6xf64>

    %3 = iqoala.transpose(%1 : tensor<6xf64>) to tensor<6xf64>
} 