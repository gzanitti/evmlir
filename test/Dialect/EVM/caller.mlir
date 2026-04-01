// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_caller() -> i256 {
  // CHECK: evm.caller
  %result = evm.caller -> i256
  return %result : i256
}
