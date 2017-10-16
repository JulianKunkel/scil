# for specific algorithms
include_directories(
	${CMAKE_SOURCE_DIR}/base
  ${CMAKE_SOURCE_DIR}/util
  ${CMAKE_BINARY_DIR}/util

  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}

	${CMAKE_CURRENT_SOURCE_DIR}/algo
	${CMAKE_CURRENT_BINARY_DIR}/algo
  ${CMAKE_CURRENT_SOURCE_DIR}/algo/util
  ${CMAKE_CURRENT_BINARY_DIR}/algo/util

  ${LIBZ_INCLUDE_DIRS}
  ${DEPS_COMPILED_DIR}/wavelet_code
  ${DEPS_COMPILED_DIR}/include/zfp
  ${DEPS_COMPILED_DIR}/include/fpzip
  ${DEPS_COMPILED_DIR}/include/sz
  ${DEPS_COMPILED_DIR}/include/
)

FILE(GLOB ALGO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/algo/*.c ${CMAKE_CURRENT_BINARY_DIR}/algo/*.c ${CMAKE_CURRENT_SOURCE_DIR}/algo/util/*.c ${CMAKE_CURRENT_BINARY_DIR}/algo/util/*.c)
FILE(GLOB REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/algo/*.dtype.c ${CMAKE_CURRENT_SOURCE_DIR}/algo/util/*.dtype.c)
list(REMOVE_ITEM ALGO_FILES ${REMOVE})

FILE(GLOB COMPRESS_FILES ${CMAKE_CURRENT_BINARY_DIR}/*.c ${CMAKE_CURRENT_SOURCE_DIR}/*.c)


add_library(scil SHARED
	${CMAKE_SOURCE_DIR}/scil-dummy.cpp
  ${ALGO_FILES}
	${COMPRESS_FILES}

  ${DEPS_COMPILED_DIR}/wavelet_code/alloc.c
  ${DEPS_COMPILED_DIR}/wavelet_code/wav_basic.c
  ${DEPS_COMPILED_DIR}/wavelet_code/wav_trf.c
)

add_dependencies(scil trigger_datatype_variants)

target_link_libraries(scil
	scil-util
	m
	${LIBZ_LIBRARIES}
  ${DEPS_COMPILED_DIR}/libfpzip.a
  ${DEPS_COMPILED_DIR}/libzfp.a
  ${DEPS_COMPILED_DIR}/libcnoise.a
  ${DEPS_COMPILED_DIR}/libsz.a
  ${DEPS_COMPILED_DIR}/liblz4.a
)

# target_link_libraries(scil INTERFACE  "-Wl,--retain-symbols-file=${CMAKE_CURRENT_SOURCE_DIR}/symbols.txt")

install(TARGETS scil LIBRARY DESTINATION lib)
install(FILES
	scil.h
	scil-context.h
	DESTINATION include)

SUBDIRS (test)
