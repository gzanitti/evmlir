// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_return() {
  %offset = arith.constant 0 : i256
  %length = arith.constant 32 : i256
  // CHECK: evm.return
  evm.return %offset, %length : i256, i256
  return
}
