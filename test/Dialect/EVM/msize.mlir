// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_msize() -> i256 {
  // CHECK: evm.msize
  %result = evm.msize -> i256
  return %result : i256
}
