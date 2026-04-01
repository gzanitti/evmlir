// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_chainid() -> i256 {
  // CHECK: evm.chainid
  %result = evm.chainid -> i256
  return %result : i256
}
