// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_bloobbasefee() -> i256 {
  // CHECK: evm.bloobbasefee
  %result = evm.bloobbasefee -> i256
  return %result : i256
}
