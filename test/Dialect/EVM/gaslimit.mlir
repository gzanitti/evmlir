// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_gaslimit() -> i256 {
  // CHECK: evm.gaslimit
  %result = evm.gaslimit -> i256
  return %result : i256
}
