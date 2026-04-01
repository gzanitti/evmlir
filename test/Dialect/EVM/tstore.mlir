// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_tstore() {
  %slot = arith.constant 0 : i256
  %value = arith.constant 42 : i256
  // CHECK: evm.tstore
  evm.tstore %slot, %value : i256, i256
  return
}
