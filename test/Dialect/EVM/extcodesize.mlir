// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_extcodesize() -> i256 {
  %address = arith.constant 0 : i256
  // CHECK: evm.extcodesize
  %result = evm.extcodesize %address : i256 -> i256
  return %result : i256
}
