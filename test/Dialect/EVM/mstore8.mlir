// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_mstore8() {
  %offset = arith.constant 0 : i256
  %value = arith.constant 255 : i256
  // CHECK: evm.mstore8
  evm.mstore8 %offset, %value : i256, i256
  return
}
