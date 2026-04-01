// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_returndatacopy() {
  %mem_offset = arith.constant 0 : i256
  %data_offset = arith.constant 0 : i256
  %len = arith.constant 32 : i256
  // CHECK: evm.returndatacopy
  evm.returndatacopy %mem_offset, %data_offset, %len : i256, i256, i256
  return
}
