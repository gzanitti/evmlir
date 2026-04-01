# Dialect 
set(LLVM_TARGET_DEFINITIONS EVMOps.td)
mlir_tablegen(EVMDialect.h.inc     -gen-dialect-decls  -dialect=evm)
mlir_tablegen(EVMDialect.cpp.inc   -gen-dialect-defs   -dialect=evm)
add_public_tablegen_target(EVMDialectIncGen)
add_mlir_doc(EVMOps evm Dialects/ -gen-dialect-doc)

# Operations
set(LLVM_TARGET_DEFINITIONS EVMOps.td)
mlir_tablegen(EVMOps.h.inc     -gen-op-decls)
mlir_tablegen(EVMOps.cpp.inc   -gen-op-defs)
add_public_tablegen_target(EVMOpsIncGen)