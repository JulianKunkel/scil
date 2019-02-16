include_directories(
	${CMAKE_SOURCE_DIR}/base
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

FILE(GLOB UTIL_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_BINARY_DIR}/*.c)
FILE(GLOB REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/*.dtype.c)
list(REMOVE_ITEM UTIL_FILES ${REMOVE})

# add files from src/base directory
FILE(GLOB CORE_FILES ${CMAKE_SOURCE_DIR}/base/*.c ${CMAKE_BINARY_DIR}/base/*.c)
add_library(scil-util SHARED
	${UTIL_FILES}
	${CORE_FILES})

target_link_libraries(scil-util
	${GCOV_LIBRARIES}
	m
	rt
)

# target_link_libraries(scil-util INTERFACE  "-Wl,--retain-symbols-file=${CMAKE_CURRENT_SOURCE_DIR}/symbols.txt")

add_dependencies(scil-util trigger_datatype_variants)

install(TARGETS scil-util LIBRARY DESTINATION lib)
install(FILES scil-util.h ${CMAKE_CURRENT_BINARY_DIR}/scil-util-types.h DESTINATION include)

SUBDIRS(test)
