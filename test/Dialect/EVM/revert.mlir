// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_revert() {
  %offset = arith.constant 0 : i256
  %length = arith.constant 32 : i256
  // CHECK: evm.revert
  evm.revert %offset, %length : i256, i256
}
