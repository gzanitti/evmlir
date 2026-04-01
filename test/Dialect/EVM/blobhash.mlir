// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_blobhash() -> i256 {
  %blob_id = arith.constant 0 : i256
  // CHECK: evm.blobhash
  %result = evm.blobhash %blob_id : i256 -> i256
  return %result : i256
}
