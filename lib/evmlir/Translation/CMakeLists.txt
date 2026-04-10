add_mlir_library(EVMTranslation
  EVMTranslation.cpp
  Analysis/LivenessInfo.cpp
  Analysis/InterferenceGraph.cpp
  StackAllocation/MemoryAllocator.cpp
  StackAllocation/StackAllocator.cpp
  CodeGen/OsakaSpec.cpp
  CodeGen/OpcodeMapper.cpp
  CodeGen/EVMCostModel.cpp
  CodeGen/EVMEmitter.cpp
  CodeGen/BytecodeStream.cpp
  CodeGen/StackTracker.cpp

  DEPENDS
  EVMDialect

  LINK_LIBS PUBLIC
  EVMDialect
  MLIRAnalysis
  MLIRFuncDialect
  MLIRIR
)