cmake_minimum_required (VERSION 2.8)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

# project name
set (PROJECT_NAME scil)
project (${PROJECT_NAME})

# version numbers
set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)

include(CTest)
include(FeatureSummary)
include(CheckCSourceCompiles)

# compile flags
set (CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -ggdb3 -std=gnu11 -pedantic -Wall -Wextra -Wdouble-promotion -Wfloat-equal -flto -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--print-gc-sections -Wl,--no-allow-shlib-undefined " # -fvisibility=hidden -fvisibility-inlines-hidden
 #-pedantic-errors
)
set ( CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -ggdb3 -pedantic -Wall -Wextra -Wdouble-promotion -Wfloat-equal -flto -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--print-gc-sections -Wl,--no-allow-shlib-undefined -Wl,--retain-symbols-file=${CMAKE_SOURCE_DIR}/symbols.txt" #-fvisibility=hidden -fvisibility-inlines-hidden
)
# set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
# set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")

set (CMAKE_C_FLAGS_DEBUG   "-O0")
set (CMAKE_C_FLAGS_RELEASE "-O3")
set (CMAKE_INSTALL_DO_STRIP TRUE)

set (SCIL_ALGO_SOURCE ${CMAKE_SOURCE_DIR}/algo)
set (SCIL_ALGO_BINARY ${CMAKE_BINARY_DIR}/algo)

include_directories(.)

# run during configure:
execute_process(COMMAND ${CMAKE_SOURCE_DIR}/deps/build-dependencies.sh WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/)
execute_process(COMMAND ${CMAKE_SOURCE_DIR}/create-datatype-variants.sh ${CMAKE_SOURCE_DIR} WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/)

FILE(GLOB ALGO ${CMAKE_SOURCE_DIR}/algo/*.c ${CMAKE_BINARY_DIR}/algo/*.c)

MESSAGE( STATUS "Available algorithms: " ${ALGO})
add_library(scil SHARED scil.c scil-util.c scil-errors.c scil-dummy.cpp ${ALGO})
target_link_libraries(scil m)

# for specific algorithms
include_directories(${CMAKE_SOURCE_DIR}/deps/include/fpzip ${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/deps/include/zfp)
target_link_libraries(scil z)
target_link_libraries(scil
  ${CMAKE_SOURCE_DIR}/deps/libfpzip.a
  ${CMAKE_SOURCE_DIR}/deps/libzfp.a
  lz4
  )

## Installation
install(TARGETS scil LIBRARY DESTINATION lib)
install(FILES scil.h DESTINATION include)

SUBDIRS (test)
