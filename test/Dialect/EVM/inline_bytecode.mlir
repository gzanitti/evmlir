// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_inline_bytecode() {
  // CHECK: evm.inline_bytecode
  evm.inline_bytecode <dense<[0x60, 0x00, 0x56]> : tensor<3xi8>>
  return
}
