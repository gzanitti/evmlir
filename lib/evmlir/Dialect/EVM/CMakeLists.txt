add_mlir_library(EVMDialect
  EVMDialect.cpp
  EVMOps.cpp

  DEPENDS
    EVMDialectIncGen
    EVMOpsIncGen

    LINK_LIBS PUBLIC
    MLIRDialect
    MLIRIR
)