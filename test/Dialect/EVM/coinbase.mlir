// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_coinbase() -> i256 {
  // CHECK: evm.coinbase
  %result = evm.coinbase -> i256
  return %result : i256
}
