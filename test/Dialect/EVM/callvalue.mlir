// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_callvalue() -> i256 {
  // CHECK: evm.callvalue
  %result = evm.callvalue -> i256
  return %result : i256
}
