configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
  MAIN_CONFIG
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg.py
)

set(EVMLIR_TEST_DEPENDS
  FileCheck count not
  evmlir-opt
)

add_lit_testsuite(check-evmlir "Running the evmlir regression tests"
  ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${EVMLIR_TEST_DEPENDS}
)

set_target_properties(check-evmlir PROPERTIES FOLDER "Tests")

add_lit_testsuites(EVMLIR ${CMAKE_CURRENT_SOURCE_DIR} DEPENDS ${EVMLIR_TEST_DEPENDS})

add_subdirectory(Translation)
