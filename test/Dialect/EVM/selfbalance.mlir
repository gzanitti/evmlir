// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_selfbalance() -> i256 {
  // CHECK: evm.selfbalance
  %result = evm.selfbalance -> i256
  return %result : i256
}
