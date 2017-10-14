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
  set_property(TEST ${TESTNAME} APPEND PROPERTY ENVIRONMENT "HDF5_PLUGIN_PATH=${CMAKE_BINARY_DIR}")
endforeach()

set_tests_properties(hints-read PROPERTIES DEPENDS hints-set)
