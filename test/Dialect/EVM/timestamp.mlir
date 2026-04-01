// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_timestamp() -> i256 {
  // CHECK: evm.timestamp
  %result = evm.timestamp -> i256
  return %result : i256
}
