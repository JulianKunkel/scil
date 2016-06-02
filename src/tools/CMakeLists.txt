project (${PROJECT_NAME})

add_executable(scil-benchmark scil-benchmark.c)
target_link_libraries(scil-benchmark scil scil-patterns)
install(TARGETS scil-benchmark RUNTIME DESTINATION bin)


add_executable(scil-pattern-plotter scil-pattern-plotter.c)
target_link_libraries(scil-pattern-plotter scil scil-patterns)
install(TARGETS scil-pattern-plotter RUNTIME DESTINATION bin)

add_executable(scil-compress-csv scil-compress-csv.c)
target_link_libraries(scil-compress-csv scil)

install(TARGETS scil-compress-csv DESTINATION bin)


install(FILES scil-plot-csv.R DESTINATION bin)
install(FILES scil-plot-csv.py DESTINATION bin)
