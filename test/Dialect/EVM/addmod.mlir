// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_addmod() -> i256 {
  %x = arith.constant 10 : i256
  %y = arith.constant 20 : i256
  %k = arith.constant 7 : i256
  // CHECK: evm.addmod
  %result = evm.addmod %x, %y, %k : i256, i256, i256 -> i256
  return %result : i256
}
