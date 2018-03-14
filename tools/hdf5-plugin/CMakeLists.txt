cmake_minimum_required (VERSION 3.0)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set (PROJECT_NAME hdf5-filter-scil)
project (${PROJECT_NAME})

set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)

find_package(PkgConfig REQUIRED)
find_package(HDF5 REQUIRED)
pkg_search_module(SCIL REQUIRED scil)
find_package(MPI REQUIRED)


include(CTest)
include(FeatureSummary)
include(CheckCSourceCompiles)


# compile flags
set (CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -ggdb3 -std=gnu11 -ldl -pedantic -Wall -Wextra -pedantic-errors"
)
set (CMAKE_C_FLAGS_DEBUG   "-O0")
set (CMAKE_C_FLAGS_RELEASE "-O3")
set (CMAKE_INSTALL_RPATH "${HDF5_LIBRARY_DIRS}" "${SCIL_LIBRARY_DIRS}")

include_directories(${CMAKE_SOURCE_DIR}/ ${CMAKE_BINARY_DIR} ${HDF5_INCLUDE_DIR} ${SCIL_INCLUDE_DIRS} ${MPI_C_INCLUDE_PATH})
add_library(hdf5-filter-scil SHARED scil-plugin.c)
target_link_libraries(hdf5-filter-scil -L${SCIL_LIBRARY_DIRS} ${HDF5_LIBRARIES} ${SCIL_LIBRARIES})


SUBDIRS (test)

## Installation
install(TARGETS hdf5-filter-scil LIBRARY DESTINATION lib)
install(FILES scil-hdf5-plugin.h DESTINATION include)
