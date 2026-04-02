set(TRANSLATION_HEADERS
  ${CMAKE_SOURCE_DIR}/lib/evmlir/Translation
)

function(add_translation_test target_name source_file)
  add_executable(${target_name} ${source_file})
  target_include_directories(${target_name} PRIVATE
    ${TRANSLATION_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}
  )
  target_link_libraries(${target_name} PRIVATE
    EVMTranslation
    GTest::gtest
    GTest::gtest_main
    MLIR
  )
  add_test(NAME ${target_name} COMMAND ${target_name})
endfunction()

add_translation_test(LivenessInfoTest      Analysis/LivenessInfo.cpp)
add_translation_test(InterferenceGraphTest Analysis/InterferenceGraph.cpp)
add_translation_test(StackAllocatorTest    StackAllocation/StackAllocator.cpp)

add_custom_target(check-evmlir-unit
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  DEPENDS LivenessInfoTest InterferenceGraphTest StackAllocatorTest
)
