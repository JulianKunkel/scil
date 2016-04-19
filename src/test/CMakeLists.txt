project (${PROJECT_NAME})

include_directories("${CMAKE_BINARY_DIR}/algo")

# Algorithm tests
add_executable(abstol-test abstol-test.c)
target_link_libraries(abstol-test scil)
target_link_libraries(abstol-test m)

# add the executable
add_executable(scil-test compression-test.c)
target_link_libraries(scil-test scil)

add_executable(scil-correctness-test correctness-test.c)
target_link_libraries(scil-correctness-test scil)

# zlib-test
add_executable(zlib-test zlib-test.c)
target_link_libraries(zlib-test scil)
target_link_libraries(zlib-test z)

# performance
add_executable(performance-test performance-test.c)
target_link_libraries(performance-test scil)
target_link_libraries(performance-test netcdf)

# Do testing
enable_testing()

add_test(gnbcf abstol-test -gnbcf)
add_test(gnbcd abstol-test -gnbcd)
add_test(rub   abstol-test -rub)
add_test(irf   abstol-test -irf)
add_test(ird   abstol-test -ird)
add_test(drf   abstol-test -drf)
add_test(drd   abstol-test -drd)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
                  DEPENDS abstol-test)
