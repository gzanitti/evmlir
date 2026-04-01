add_mlir_library(EVMTranslation
  EVMTranslation.cpp
  Analysis/LivenessInfo.cpp
  Analysis/InterferenceGraph.cpp
  StackAllocation/StackAllocator.cpp
  CodeGen/EVMEmitter.cpp

  DEPENDS
  EVMDialect

  LINK_LIBS PUBLIC
  EVMDialect
  MLIRAnalysis
  MLIRFuncDialect
  MLIRIR
)