// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_log1() {
  %data_offset = arith.constant 0 : i256
  %data_length = arith.constant 32 : i256
  %topic1 = arith.constant 1 : i256
  // CHECK: evm.log1
  evm.log1 %data_offset, %data_length, %topic1 : i256, i256, i256
  return
}
