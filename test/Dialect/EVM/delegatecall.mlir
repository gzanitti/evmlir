// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_delegatecall() -> i256 {
  %gas = arith.constant 0 : i256
  %to = arith.constant 0 : i256
  %args_offset = arith.constant 0 : i256
  %args_length = arith.constant 0 : i256
  %ret_offset = arith.constant 0 : i256
  %ret_length = arith.constant 0 : i256
  // CHECK: evm.delegatecall
  %result = evm.delegatecall %gas, %to, %args_offset, %args_length, %ret_offset, %ret_length : i256, i256, i256, i256, i256, i256 -> i256
  return %result : i256
}
