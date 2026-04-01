// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_basefee() -> i256 {
  // CHECK: evm.basefee
  %result = evm.basefee -> i256
  return %result : i256
}
