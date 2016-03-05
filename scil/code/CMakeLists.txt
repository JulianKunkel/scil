cmake_minimum_required (VERSION 2.8)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

# project name
set (PROJECT_NAME scil)
project (${PROJECT_NAME})

# version numbers
set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)

# compile flags
set (CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -ggdb3 -std=gnu11 -pedantic -Wall -Wextra -Wconversion -Wdouble-promotion -Wfloat-equal"
)
# set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
# set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")

set (CMAKE_C_FLAGS_DEBUG "-O0")
set (CMAKE_C_FLAGS_RELEASE "-O3")


include_directories(.)


FILE(GLOB ALGO algo/*.c)

MESSAGE( STATUS "Available algorithms: " ${ALGO})

add_library(scil SHARED scil.c scil-util.c ${ALGO})
target_link_libraries(scil m)
target_link_libraries(scil z)

SUBDIRS (test)

## Installation
install(TARGETS scil LIBRARY DESTINATION lib)
install(FILES scil.h DESTINATION include)
