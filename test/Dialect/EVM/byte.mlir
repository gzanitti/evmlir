// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_byte() -> i256 {
  %index = arith.constant 31 : i256
  %x = arith.constant 255 : i256
  // CHECK: evm.byte
  %result = evm.byte %index, %x : i256, i256 -> i256
  return %result : i256
}
