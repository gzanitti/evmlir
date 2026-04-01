// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_codesize() -> i256 {
  // CHECK: evm.codesize
  %result = evm.codesize -> i256
  return %result : i256
}
