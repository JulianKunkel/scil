project (${PROJECT_NAME})

add_executable(scil-benchmark scil-benchmark.c)
target_link_libraries(scil-benchmark scil scil-patterns)
install(TARGETS scil-benchmark RUNTIME DESTINATION bin)
