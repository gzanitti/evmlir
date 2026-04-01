// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_balance() -> i256 {
  %address = arith.constant 0 : i256
  // CHECK: evm.balance
  %result = evm.balance %address : i256 -> i256
  return %result : i256
}
