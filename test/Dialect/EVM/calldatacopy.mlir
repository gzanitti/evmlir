// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_calldatacopy() {
  %mem_offset = arith.constant 0 : i256
  %cd_offset = arith.constant 0 : i256
  %len = arith.constant 32 : i256
  // CHECK: evm.calldatacopy
  evm.calldatacopy %mem_offset, %cd_offset, %len : i256, i256, i256
  return
}
