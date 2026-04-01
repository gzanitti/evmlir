// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_pc() -> i256 {
  // CHECK: evm.pc
  %result = evm.pc -> i256
  return %result : i256
}
