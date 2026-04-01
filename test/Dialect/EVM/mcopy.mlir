// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_mcopy() {
  %dest = arith.constant 0 : i256
  %src = arith.constant 32 : i256
  %len = arith.constant 64 : i256
  // CHECK: evm.mcopy
  evm.mcopy %dest, %src, %len : i256, i256, i256
  return
}
