// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_calldatasize() -> i256 {
  // CHECK: evm.calldatasize
  %result = evm.calldatasize -> i256
  return %result : i256
}
