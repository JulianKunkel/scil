project (${PROJECT_NAME})

include_directories(
	${CMAKE_SOURCE_DIR}/core
  ${CMAKE_SOURCE_DIR}/util
  ${CMAKE_BINARY_DIR}/util
  ${DEPS_DIR}/open-simplex-noise-in-c
  ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB PATTERNFILES "${CMAKE_CURRENT_SOURCE_DIR}" "*.c")

add_library(scil-patterns SHARED
  ${PATTERNFILES}
  ${DEPS_DIR}/open-simplex-noise-in-c/open-simplex-noise.c
)

target_link_libraries(scil-patterns
  scil-util
  m)

install(TARGETS scil-patterns LIBRARY DESTINATION lib)
install(FILES scil-patterns.h DESTINATION include)
