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


include_directories(argo .)

set (SOURCE
	main.c
    algo/algo-1.c
    algo/memcopy.c
	util.c
)

add_library(scil SHARED scil.c)
target_link_libraries(scil m)

# add the executable
add_executable(scil-test ${SOURCE})
target_link_libraries(scil-test scil)

# algo-1-test
add_executable(algo-1-test algo/algo-1-test.c)
target_link_libraries(algo-1-test m)

## Installation
install(TARGETS scil LIBRARY DESTINATION lib)
install(FILES scil.h DESTINATION include)