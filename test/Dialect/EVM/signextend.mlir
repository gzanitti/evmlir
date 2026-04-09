// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_signextend() -> i256 {
  %byte = arith.constant 0 : i256
  %x = arith.constant 255 : i256
  // CHECK: evm.signextend
  %result = evm.signextend %byte, %x : i256, i256 -> i256
  return %result : i256
}
