// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_log4() {
  %data_offset = arith.constant 0 : i256
  %data_length = arith.constant 32 : i256
  %topic1 = arith.constant 1 : i256
  %topic2 = arith.constant 2 : i256
  %topic3 = arith.constant 3 : i256
  %topic4 = arith.constant 4 : i256
  // CHECK: evm.log4
  evm.log4 %data_offset, %data_length, %topic1, %topic2, %topic3, %topic4 : i256, i256, i256, i256, i256, i256
  return
}
