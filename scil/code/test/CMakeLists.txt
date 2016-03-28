project (${PROJECT_NAME})

# add the executable
add_executable(scil-test compression-test.c)
target_link_libraries(scil-test scil)

# algo-1-test
add_executable(algo-1-test algo-1-test.c)
target_link_libraries(algo-1-test m)

# zlib-test
add_executable(zlib-test zlib-test.c ../scil-util.c)
target_link_libraries(zlib-test z)

# performance
add_executable(performance-test performance-test.c)
target_link_libraries(performance-test scil)
target_link_libraries(performance-test netcdf)
