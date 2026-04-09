// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_invalid() {
  // CHECK: evm.invalid
  evm.invalid
}
