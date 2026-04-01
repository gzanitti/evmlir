// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_address() -> i256 {
  // CHECK: evm.address
  %result = evm.address -> i256
  return %result : i256
}
