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
    "${CMAKE_C_FLAGS} -ggdb3 -std=gnu11 -pedantic -Wall -Wextra -Wconversion -Wdouble-promotion -Wfloat-equal -lm"
)
# set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
# set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
set (CMAKE_C_FLAGS_DEBUG "-O0")
set (CMAKE_C_FLAGS_RELEASE "-O3")

set (SOURCE
	main.c
	scil.c
	util.c
)

# add the executable
add_executable(scil ${SOURCE})