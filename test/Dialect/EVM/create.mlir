// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_create() -> i256 {
  %value = arith.constant 0 : i256
  %init_code_offset = arith.constant 0 : i256
  %init_code_length = arith.constant 32 : i256
  // CHECK: evm.create
  %result = evm.create %value, %init_code_offset, %init_code_length : i256, i256, i256 -> i256
  return %result : i256
}
