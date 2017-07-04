project(tests C)

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMOCKA_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/include
)

set(CMOCKA_TESTS
    test_branches)
foreach(_CMOCKA_TEST ${CMOCKA_TESTS})
    add_cmocka_test(${_CMOCKA_TEST} ${_CMOCKA_TEST}.c ${CMOCKA_BRANCHES_STATIC_LIBRARY} ${CMOCKA_LIBRARY})
endforeach()


