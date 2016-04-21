project (${PROJECT_NAME})

file(GLOB TESTFILES "*.c")
MESSAGE (${TESTFILES})
foreach(TESTFILE ${TESTFILES})
  get_filename_component(TESTNAME ${TESTFILE} NAME)
  if ( NOT ${TESTNAME} MATCHES "test" )
	  add_executable(test-${TESTNAME} ${TESTFILE})
	  target_link_libraries(test-${TESTNAME} scil)

	  add_test(${TESTNAME} ./test-${TESTNAME})
  endif()
endforeach()

# add the executable
# add_executable(scil-test compression-test.c)
# target_link_libraries(scil-test scil)

# add_executable(scil-correctness-test correctness-test.c)
# target_link_libraries(scil-correctness-test scil)

# abstol-test
# add_executable(abstol-test abstol-test.c)
# target_link_libraries(abstol-test scil)

# zlib-test
# add_executable(zlib-test zlib-test.c)
# target_link_libraries(zlib-test scil)

# performance
# add_executable(performance-test performance-test.c)
# target_link_libraries(performance-test scil)
# target_link_libraries(performance-test netcdf)
