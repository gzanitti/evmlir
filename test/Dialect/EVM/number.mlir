// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_number() -> i256 {
  // CHECK: evm.number
  %result = evm.number -> i256
  return %result : i256
}
