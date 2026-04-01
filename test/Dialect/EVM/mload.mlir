// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_mload() -> i256 {
  %offset = arith.constant 0 : i256
  // CHECK: evm.mload
  %result = evm.mload %offset : i256 -> i256
  return %result : i256
}
