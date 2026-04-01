// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_mstore() {
  %offset = arith.constant 0 : i256
  %value = arith.constant 42 : i256
  // CHECK: evm.mstore
  evm.mstore %offset, %value : i256, i256
  return
}
