// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_extcodehash() -> i256 {
  %address = arith.constant 0 : i256
  // CHECK: evm.extcodehash
  %result = evm.extcodehash %address : i256 -> i256
  return %result : i256
}
