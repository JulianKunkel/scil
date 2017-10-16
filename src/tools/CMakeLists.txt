include_directories(
	${CMAKE_SOURCE_DIR}/base
  ${CMAKE_SOURCE_DIR}/compression
  ${CMAKE_SOURCE_DIR}/pattern
  ${CMAKE_SOURCE_DIR}/util
  ${CMAKE_BINARY_DIR}/util

  ${DEPS_COMPILED_DIR}/include/cnoise
  ${CMAKE_CURRENT_SOURCE_DIR}/util
)

add_executable(scil-benchmark scil-benchmark.c)
target_link_libraries(scil-benchmark scil scil-patterns scil-tools-util)
install(TARGETS scil-benchmark RUNTIME DESTINATION bin)

add_executable(scil-pattern-creator scil-pattern-creator.c)
target_link_libraries(scil-pattern-creator scil scil-patterns scil-tools-util)
install(TARGETS scil-pattern-creator RUNTIME DESTINATION bin)

add_executable(scil-compress scil-compress.c)
target_link_libraries(scil-compress scil scil-tools-util)

add_executable(scil-add-noise scil-add-noise.c)
target_link_libraries(scil-add-noise scil gsl gslcblas m scil-tools-util)

add_executable(scil-generate-chooser-data scil-generate-chooser-data.c)
target_link_libraries(scil-generate-chooser-data scil scil-patterns scil-tools-util)

add_executable(scil-generate-chooser-data2 scil-generate-chooser-data2.c)
target_link_libraries(scil-generate-chooser-data2 scil scil-patterns scil-tools-util)

install(TARGETS scil-compress DESTINATION bin)

install(FILES scil-plot-csv.R DESTINATION bin)
install(FILES scil-plot-csv.py DESTINATION bin)

SUBDIRS (util)
