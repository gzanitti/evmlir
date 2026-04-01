add_mlir_library(EVMTransforms
  EVMCanonicalize.cpp

  DEPENDS
    EVMOpsIncGen
    EVMTransformsPassIncGen

  LINK_LIBS PUBLIC
    EVMDialect
    MLIRFuncDialect
    MLIRPass
    MLIRTransforms
)