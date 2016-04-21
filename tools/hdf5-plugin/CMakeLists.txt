cmake_minimum_required (VERSION 3.2)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set (PROJECT_NAME hdf5-filter-scil)
project (${PROJECT_NAME})

set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)

find_package(PkgConfig REQUIRED)
find_package(HDF5 REQUIRED)
pkg_search_module(SCIL REQUIRED scil)


include(CTest)
include(FeatureSummary)
include(CheckCSourceCompiles)

# compile flags
set (CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -ggdb3 -std=gnu11 -pedantic -Wall -Wextra -pedantic-errors -Wl,--rpath=${HDF5_LIBRARY_DIRS}:${SCIL_LIBRARY_DIRS}"
)
set (CMAKE_C_FLAGS_DEBUG   "-O0")
set (CMAKE_C_FLAGS_RELEASE "-O3")

include_directories(${CMAKE_SOURCE_DIR}/ ${CMAKE_BINARY_DIR} ${HDF5_INCLUDE_DIR} ${SCIL_INCLUDE_DIRS})
add_library(hdf5-filter-scil SHARED scil-plugin.c)
target_link_libraries(hdf5-filter-scil -L${HDF5_LIBRARY_DIRS} -L${SCIL_LIBRARY_DIRS} ${HDF5_LIBRARIES} ${SCIL_LIBRARIES})

SUBDIRS (test)

## Installation
install(TARGETS hdf5-filter-scil LIBRARY DESTINATION lib)