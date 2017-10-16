project (${PROJECT_NAME})

include_directories(
  ${CMAKE_SOURCE_DIR}/base
  ${CMAKE_SOURCE_DIR}/util
  ${CMAKE_BINARY_DIR}/util
  ${CMAKE_SOURCE_DIR}/compression
  ${CMAKE_SOURCE_DIR}/pattern
)

file(GLOB TESTFILES "${CMAKE_CURRENT_SOURCE_DIR}" "*.c")
foreach(TESTFILE ${TESTFILES})
  if(IS_DIRECTORY ${TESTFILE} )
    #message(STATUS ${TESTFILE})
  else()
    get_filename_component(TESTNAME_C ${TESTFILE} NAME)
    STRING(REGEX REPLACE ".c$" "" TESTNAME ${TESTNAME_C})
      add_executable(${TESTNAME}.exe ${TESTFILE})
    target_compile_options(${TESTNAME}.exe PUBLIC -DDEBUG )
    target_link_libraries(${TESTNAME}.exe scil scil-patterns)

    add_test(${TESTNAME} ./${TESTNAME}.exe)
  endif()

endforeach()

#SUBDIRS (complex)
