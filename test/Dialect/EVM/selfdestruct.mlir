// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_selfdestruct() {
  %recipient = arith.constant 0 : i256
  // CHECK: evm.selfdestruct
  evm.selfdestruct %recipient : i256
  return
}
