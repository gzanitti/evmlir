// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_gas() -> i256 {
  // CHECK: evm.gas
  %result = evm.gas -> i256
  return %result : i256
}
