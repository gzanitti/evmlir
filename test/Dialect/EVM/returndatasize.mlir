// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_returndatasize() -> i256 {
  // CHECK: evm.returndatasize
  %result = evm.returndatasize -> i256
  return %result : i256
}
