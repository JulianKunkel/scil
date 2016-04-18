project (${PROJECT_NAME})

file(GLOB_RECURSE TESTFILES "${CMAKE_CURRENT_SOURCE_DIR}" "*.c")
foreach(TESTFILE ${TESTFILES})
  get_filename_component(TESTNAME ${TESTFILE} NAME)
  add_executable(test-${TESTNAME} ${TESTFILE})
  target_link_libraries(test-${TESTNAME} scil)

  add_test(${TESTNAME} ./test-${TESTNAME})
endforeach()


SUBDIRS (complex)
