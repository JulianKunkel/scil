project (${PROJECT_NAME})

file(GLOB TESTFILES "${CMAKE_CURRENT_SOURCE_DIR}" "*.c")
foreach(TESTFILE ${TESTFILES})
  get_filename_component(TESTNAME_C ${TESTFILE} NAME)
  if(EXISTS "${TESTFILE}" AND IS_DIRECTORY "${TESTFILE}")
    continue()
  endif()
  STRING(REGEX REPLACE ".c$" "" TESTNAME ${TESTNAME_C})

  add_executable(${TESTNAME}.exe ${TESTFILE})
  target_link_libraries(${TESTNAME}.exe hdf5-filter-scil)

  add_test(${TESTNAME} ./${TESTNAME}.exe)
endforeach()
