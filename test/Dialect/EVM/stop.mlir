// RUN: %evmlir-opt %s | FileCheck %s

func.func @test_stop() {
  // CHECK: evm.stop
  evm.stop
}