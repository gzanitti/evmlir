// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_gasprice() -> i256 {
  // CHECK: evm.gasprice
  %result = evm.gasprice -> i256
  return %result : i256
}
