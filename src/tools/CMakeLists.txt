set(FILE_PLUGINS "plugins/file-csv.c" "plugins/file-plugin.c" "plugins/file-bin.c")

add_executable(scil-benchmark scil-benchmark.c)
target_link_libraries(scil-benchmark scil scil-patterns)
install(TARGETS scil-benchmark RUNTIME DESTINATION bin)


add_executable(scil-pattern-plotter scil-pattern-plotter.c)
target_link_libraries(scil-pattern-plotter scil scil-patterns)
install(TARGETS scil-pattern-plotter RUNTIME DESTINATION bin)

add_executable(scil-compress scil-compress.c ${FILE_PLUGINS})
target_link_libraries(scil-compress scil)

add_executable(scil-add-noise scil-add-noise.c ${FILE_PLUGINS})
target_link_libraries(scil-add-noise scil gsl gslcblas m)

add_executable(scil-generate-chooser-data scil-generate-chooser-data.c)
target_link_libraries(scil-generate-chooser-data scil scil-patterns)

add_executable(scil-generate-chooser-data2 scil-generate-chooser-data2.c)
target_link_libraries(scil-generate-chooser-data2 scil scil-patterns)

install(TARGETS scil-compress DESTINATION bin)

install(FILES scil-plot-csv.R DESTINATION bin)
install(FILES scil-plot-csv.py DESTINATION bin)
