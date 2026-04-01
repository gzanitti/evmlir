// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_keccak256() -> i256 {
  %offset = arith.constant 0 : i256
  %len = arith.constant 32 : i256
  // CHECK: evm.keccak256
  %result = evm.keccak256 %offset, %len : i256, i256 -> i256
  return %result : i256
}
