// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_exp() -> i256 {
  %base = arith.constant 2 : i256
  %exponent = arith.constant 10 : i256
  // CHECK: evm.exp
  %result = evm.exp %base, %exponent : i256, i256 -> i256
  return %result : i256
}
