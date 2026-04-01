// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_sstore() {
  %slot = arith.constant 0 : i256
  %value = arith.constant 42 : i256
  // CHECK: evm.sstore
  evm.sstore %slot, %value : i256, i256
  return
}
