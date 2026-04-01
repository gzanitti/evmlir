// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_blockhash() -> i256 {
  %block_number = arith.constant 0 : i256
  // CHECK: evm.blockhash
  %result = evm.blockhash %block_number : i256 -> i256
  return %result : i256
}
