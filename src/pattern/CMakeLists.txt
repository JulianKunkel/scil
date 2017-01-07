project (${PROJECT_NAME})

file(GLOB PATTERNFILES "${CMAKE_CURRENT_SOURCE_DIR}" "*.c")
add_library(scil-patterns SHARED ${PATTERNFILES})
target_link_libraries(scil-patterns m ${GCOV_LIBRARIES})

install(TARGETS scil-patterns LIBRARY DESTINATION lib)
install(FILES scil-patterns.h DESTINATION include)
