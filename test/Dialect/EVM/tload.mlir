// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_tload() -> i256 {
  %slot = arith.constant 0 : i256
  // CHECK: evm.tload
  %result = evm.tload %slot : i256 -> i256
  return %result : i256
}
