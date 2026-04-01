// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_sload() -> i256 {
  %slot = arith.constant 0 : i256
  // CHECK: evm.sload
  %result = evm.sload %slot : i256 -> i256
  return %result : i256
}
