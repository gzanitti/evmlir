// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_create2() -> i256 {
  %value = arith.constant 0 : i256
  %init_code_offset = arith.constant 0 : i256
  %init_code_length = arith.constant 32 : i256
  %salt = arith.constant 1 : i256
  // CHECK: evm.create2
  %result = evm.create2 %value, %init_code_offset, %init_code_length, %salt : i256, i256, i256, i256 -> i256
  return %result : i256
}
