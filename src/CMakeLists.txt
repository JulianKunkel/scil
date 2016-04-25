cmake_minimum_required (VERSION 2.8)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

# project name
set (PROJECT_NAME scil)
project (${PROJECT_NAME})

# version numbers
set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)

set(DEV_DIR "${CMAKE_SOURCE_DIR}/../dev")
set(DEPS_DIR "${CMAKE_SOURCE_DIR}/../deps")

include(CTest)
include(FeatureSummary)
include(CheckCSourceCompiles)

# compile flags
set (CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -pedantic-errors -std=gnu11 -pedantic -Wall -Wextra -Wdouble-promotion -Wfloat-equal -flto -fdata-sections -ffunction-sections -Wl,--gc-sections  -Wl,--no-allow-shlib-undefined")
# -fvisibility=hidden -fvisibility-inlines-hidden -Wl,--print-gc-sections
set ( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wdouble-promotion -Wfloat-equal -flto -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--no-allow-shlib-undefined -Wl,--retain-symbols-file=${CMAKE_SOURCE_DIR}/symbols.txt")

set (CMAKE_C_FLAGS_DEBUG   "-O0 -ggdb -DDEBUG")
set (CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb")

set (CMAKE_C_FLAGS_RELEASE "-O3")

set (SCIL_ALGO_SOURCE ${CMAKE_SOURCE_DIR}/algo)
set (SCIL_ALGO_BINARY ${CMAKE_BINARY_DIR}/algo)

include_directories(.)

# run during configure:
execute_process(COMMAND ${DEPS_DIR}/build-dependencies.sh WORKING_DIRECTORY ${DEPS_DIR})
execute_process(COMMAND ${DEV_DIR}/create-datatype-variants.sh ${CMAKE_SOURCE_DIR} WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/)

FILE(GLOB ALGO ${CMAKE_SOURCE_DIR}/algo/*.c ${CMAKE_BINARY_DIR}/algo/*.c)

MESSAGE( STATUS "Available algorithms: " ${ALGO})
add_library(scil SHARED scil.c scil-util.c scil-errors.c scil-dummy.cpp scil-algo-chooser.c scil-hardware-limits.c ${ALGO})
target_link_libraries(scil m)

# for specific algorithms
include_directories(${DEPS_DIR}/include/fpzip ${CMAKE_BINARY_DIR})
include_directories(${DEPS_DIR}/include/zfp)
target_link_libraries(scil z)
target_link_libraries(scil
  ${DEPS_DIR}/libfpzip.a
  ${DEPS_DIR}/libzfp.a
  lz4
  )

configure_file("${CMAKE_SOURCE_DIR}/scil-config.h.in" "scil-config.h" @ONLY)

## Installation
configure_file("${DEV_DIR}/scil.pc.in" "scil.pc" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/scil.pc"  DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")
install(TARGETS scil LIBRARY DESTINATION lib)
install(FILES scil.h DESTINATION include)

SUBDIRS (test)
