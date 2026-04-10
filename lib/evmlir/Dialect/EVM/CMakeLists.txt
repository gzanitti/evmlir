add_mlir_library(EVMDialect
  EVMDialect.cpp
  EVMOps.cpp

  DEPENDS
    EVMDialectIncGen
    EVMOpsIncGen
    EVMAttributesIncGen

    LINK_LIBS PUBLIC
    MLIRDialect
    MLIRIR
)