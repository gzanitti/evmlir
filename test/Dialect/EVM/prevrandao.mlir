// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_prevrandao() -> i256 {
  // CHECK: evm.prevrandao
  %result = evm.prevrandao -> i256
  return %result : i256
}
