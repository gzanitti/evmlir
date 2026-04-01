// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_extcodecopy() {
  %address = arith.constant 0 : i256
  %mem_offset = arith.constant 0 : i256
  %code_offset = arith.constant 0 : i256
  %len = arith.constant 32 : i256
  // CHECK: evm.extcodecopy
  evm.extcodecopy %address, %mem_offset, %code_offset, %len : i256, i256, i256, i256
  return
}
