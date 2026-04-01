set(LLVM_LINK_COMPONENTS
  Support  
)

add_llvm_executable(evmlir-opt
  evmlir-opt.cpp
)

target_link_libraries(evmlir-opt
  PRIVATE
  EVMDialect
  EVMTransforms
  MLIROptLib
  MLIR
)