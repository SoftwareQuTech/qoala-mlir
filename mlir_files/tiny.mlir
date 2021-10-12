module  {
  func @main() {
    %0 = qnir.constant dense<5.500000e+00> : tensor<f64>
    qnir.print %0 : tensor<f64>
    qnir.return
  }
}
