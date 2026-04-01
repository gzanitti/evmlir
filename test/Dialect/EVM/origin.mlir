// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_origin() -> i256 {
  // CHECK: evm.origin
  %result = evm.origin -> i256
  return %result : i256
}
