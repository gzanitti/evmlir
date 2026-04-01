// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_log0() {
  %data_offset = arith.constant 0 : i256
  %data_length = arith.constant 32 : i256
  // CHECK: evm.log0
  evm.log0 %data_offset, %data_length : i256, i256
  return
}
