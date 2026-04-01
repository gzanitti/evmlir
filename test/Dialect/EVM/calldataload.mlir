// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_calldataload() -> i256 {
  %offset = arith.constant 0 : i256
  // CHECK: evm.calldataload
  %result = evm.calldataload %offset : i256 -> i256
  return %result : i256
}
